#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 0x200

static int start[2] = {0x7400, 0x8200};

void print_info(Elf32_Phdr *phdr, int i);

Elf32_Phdr * read_exec_file(FILE **execfile, char *filename, Elf32_Ehdr **ehdr)
{
	size_t numread, phnum;
	Elf32_Phdr *phdr;
	*execfile = fopen(filename, "rb");
	assert(*execfile);

	numread = fread(*ehdr, 1, sizeof(Elf32_Ehdr), *execfile);
	phnum = (*ehdr)->e_phnum;

	fseek(*execfile, (*ehdr)->e_phoff, SEEK_SET);
	phdr = calloc(phnum, sizeof(Elf32_Phdr));//????

	numread = fread(phdr, sizeof(Elf32_Phdr), phnum, *execfile);

	return phdr;
}
void write_image(FILE **imagefile, FILE *this_file, Elf32_Ehdr *this_header, Elf32_Phdr *this_phdr){
	//write file to image
	int phi;//record the segment we are on
	size_t zero_num;//num of zero to be filled
	size_t code_num;//num of code to be filled
	size_t numread, numunread;
	size_t readsum = 0;
	char buf[SECTOR_SIZE];

	for(phi = 0; phi < this_header->e_phnum; phi++){
		code_num = this_phdr[phi].p_filesz;
		zero_num = this_phdr[phi].p_memsz - code_num;
		fseek(this_file, this_phdr[phi].p_offset, SEEK_SET);
		//copy the code
		for(; code_num > 0; code_num-=numread){
			numunread = (code_num < SECTOR_SIZE) ? code_num : SECTOR_SIZE;
			numread = fread(buf, 1, numunread, this_file);
			assert(numread == numunread);

			numread = fwrite(buf, 1, numunread, *imagefile);
			assert(numread == numunread);

			readsum += numread;
		}
		//fill 0 for the segment
		for(; zero_num > 0; zero_num--){
			fputc(0, *imagefile);
			readsum++;
		}
	}
	//fill 0 to the next sector
	zero_num = (SECTOR_SIZE - readsum) % SECTOR_SIZE;
	for(; zero_num > 0; zero_num--)
		fputc(0, *imagefile);
}

void write_bootblock(FILE **imagefile, FILE *boot_file, Elf32_Ehdr *boot_header,
		Elf32_Phdr *boot_phdr)
{
	fseek(*imagefile, 0, SEEK_SET);
	//write bootblock
	write_image(imagefile, boot_file, boot_header, boot_phdr);

	//signature for bootblock
	fseek(*imagefile, 0x1fe, SEEK_SET);
	fputc(0x55, *imagefile);
	fputc(0xaa, *imagefile);
}

void write_kernel(FILE **image_file, FILE *kernel_file, Elf32_Ehdr *kernel_ehdr, Elf32_Phdr *kernel_phdr)
{
	fseek(*image_file, SECTOR_SIZE, SEEK_SET);
	//write kernel
	write_image(image_file, kernel_file, kernel_ehdr, kernel_phdr);
}

void write_process(int i, FILE **imagefile, FILE *process_file,
		Elf32_Ehdr *process_ehdr, Elf32_Phdr *process_phdr){
	fseek(*imagefile, start[i-1], SEEK_SET);
	//write process
	write_image(imagefile, process_file, process_ehdr, process_phdr);
}

int count_kernel_sectors(Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr){
	int phi;
	size_t file_size = 0;
	for(phi=0; phi < kernel_header->e_phnum; phi++){
		file_size += kernel_phdr[phi].p_memsz;
	}
	return (int)file_size/SECTOR_SIZE + ((file_size % SECTOR_SIZE == 0)? 0 : 1);
}

int count_sectors(Elf32_Ehdr *process_header, Elf32_Phdr *process_phdr){
	int phi;
	size_t file_size = 0;
	for(phi=0; phi < process_header->e_phnum; phi++){
		file_size += process_phdr[phi].p_memsz;
	}
	return (int)file_size/SECTOR_SIZE + 384 +((file_size % SECTOR_SIZE == 0)?0 : 1);
}

void record_kernel_sectors(FILE **image_file, Elf32_Ehdr *kernel_ehdr, Elf32_Phdr *kernel_phdr, int num_sec){
	fseek(*image_file, 0x80, SEEK_SET);
	//num_sec = 0x200;
	fwrite(&num_sec, sizeof(int), 1, *image_file);
}

void extended_opt(Elf32_Phdr *boot_phdr, int k_phnum, Elf32_Phdr *kernel_phdr, int num_sec,
	FILE *bootfile, FILE *kernelfile){
	int phi;
	long int boot_length, kernel_length;
	fseek(bootfile, 0, SEEK_END);
	boot_length = ftell(bootfile);
	fseek(kernelfile, 0, SEEK_END);
	kernel_length = ftell(kernelfile);
	printf("bootblock image info\n");
	printf("length of bootblock: %ld\n", boot_length);
	printf("sectors: %d\n", 1);
	print_info(boot_phdr, 0);
	printf("kernel image info\n");
	printf("length of kernel: %d\n", kernel_length);
	printf("sectors: %d\n", num_sec-1);
	for(phi = 0; phi<k_phnum; phi++){
		printf("info of segment%d of kernel\n", phi+1);
		print_info(kernel_phdr, phi);
	}
}

void print_info(Elf32_Phdr *phdr, int i){
	printf("offset of segment in the file: 0x%4x\n", phdr[i].p_offset);
	printf("the image's virtual address of segment in memory: 0x%8x\n", phdr[i].p_vaddr);
	printf("the file image size of segment: 0x%4x\n", phdr[i].p_filesz);
	printf("the memory image size of segment: 0x%4x\n", phdr[i].p_memsz);
	printf("the align of image: 0x%4x\n",phdr[i].p_align);
}

int main(int argc, char *argv[]){
	Elf32_Phdr *boot_phdr;
	Elf32_Phdr *kernel_phdr;
	Elf32_Phdr *process1_phdr;
	Elf32_Phdr *process2_phdr;
	//Elf32_Phdr *process3_phdr;

	Elf32_Ehdr *boot_ehdr = malloc(sizeof(Elf32_Ehdr));
	Elf32_Ehdr *kernel_ehdr = malloc(sizeof(Elf32_Ehdr));
	Elf32_Ehdr *process1_ehdr = malloc(sizeof(Elf32_Ehdr));
	Elf32_Ehdr *process2_ehdr = malloc(sizeof(Elf32_Ehdr));
	//Elf32_Ehdr *process3_ehdr = malloc(sizeof(Elf32_Ehdr));

	FILE *bootp, *kernelp, *imagep, *process1p, *process2p;
	int num_sec;
	imagep = fopen("./image", "w+");
	assert(imagep);

	boot_phdr = read_exec_file(&bootp, argv[argc-4], &boot_ehdr);
	write_bootblock(&imagep, bootp, boot_ehdr, boot_phdr);

	kernel_phdr = read_exec_file(&kernelp, argv[argc-3], &kernel_ehdr);
	write_kernel(&imagep, kernelp, kernel_ehdr, kernel_phdr);

	process1_phdr = read_exec_file(&process1p, argv[argc-2], &process1_ehdr);
	write_process(1, &imagep, process1p, process1_ehdr, process1_phdr);

	process2_phdr = read_exec_file(&process2p, argv[argc-1], &process2_ehdr);
	write_process(2, &imagep, process2p, process2_ehdr, process2_phdr);

	//process3_phdr = read_exec_file(&process3p, argv[argc-1], &process3_ehdr);
	//write_process(3, &imagep, process3p, process3_ehdr, process3_phdr);

	num_sec = count_sectors(process2_ehdr, process2_phdr);
	record_kernel_sectors(&imagep, kernel_ehdr, kernel_phdr, num_sec);

	if(!strncmp(argv[1], "--extended", 11)){
		extended_opt(boot_phdr, (int)(*kernel_ehdr).e_phnum, kernel_phdr, num_sec+1, bootp, kernelp);
	}

	free(boot_ehdr);
	free(kernel_ehdr);
	free(process1_ehdr);
	free(process2_ehdr);
	//free(process3_ehdr);

	free(boot_phdr);
	free(kernel_phdr);
	free(process1_phdr);
	free(process2_phdr);
	//free(process3_phdr);

	fclose(bootp);
	fclose(kernelp);
	fclose(process1p);
	fclose(process2p);
	//fclose(process3p);
	fclose(imagep);
}
