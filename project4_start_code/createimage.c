#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Elf32_Phdr * read_exec_file(FILE **execfile, char *filename, Elf32_Ehdr **ehdr)
{
	Elf32_Phdr *phdr;
	int e_phnum;
	*execfile = fopen(filename, "r");
	fread(*ehdr, sizeof(Elf32_Ehdr), 1, *execfile);

	e_phnum = (*ehdr)->e_phnum;
	phdr = calloc(sizeof(Elf32_Phdr), e_phnum);
	fread(phdr, sizeof(Elf32_Phdr), e_phnum, *execfile);
	return phdr;
}

void write_bootblock(FILE **image_file, FILE *boot_file, Elf32_Ehdr *boot_header,Elf32_Phdr *boot_phdr)
{
	char boot[512];
	fseek(*image_file, 0, SEEK_SET);
	fseek(boot_file, (*boot_phdr).p_offset, SEEK_SET);
	fread(boot, (*boot_phdr).p_filesz, 1, boot_file);
	fwrite(boot, 1, (*boot_phdr).p_filesz, *image_file);
	fseek(*image_file, 0x1fe, SEEK_SET);
	fputc(0x55, *image_file);
	fputc(0xaa, *image_file);
}

void write_kernel(FILE **image_file, FILE *kernel_file, Elf32_Ehdr *kernel_ehdr, Elf32_Phdr *kernel_phdr)
{
	int i;
	char kernel[100000];
	fseek(*image_file, 512, SEEK_SET);
	for(i=0;i<kernel_ehdr->e_phnum;i++) {
		fseek(kernel_file, kernel_phdr[i].p_offset, SEEK_SET);
		fread(kernel, (*kernel_phdr).p_filesz, 1, kernel_file);
	}
}

void write_process(FILE **image_file, FILE *process_file, Elf32_Ehdr *process_ehdr, Elf32_Phdr *process_phdr, int process_sector)
{
	int i;
	char process[10000];
	fseek(*image_file, 512*process_sector, SEEK_SET);
	for(i=0;i<process_ehdr->e_phnum;i++) {
		fseek(process_file, process_phdr[i].p_offset, SEEK_SET);
		fread(process, (*process_phdr).p_filesz, 1, process_file);
		fwrite(process, process_phdr[i].p_filesz, 1, *image_file);
	}
}


//change function name from "count_kernel_sectors" to "count_sectors" to count process's sectors number.
int count_sectors(Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr){
	int i;
	int  sum = 0;
	for(i=0;i<kernel_header->e_phnum;i++)
		sum += kernel_phdr[i].p_memsz;
	if(sum%512!=0) return sum/512+1;
	else return sum/512;
}

void record_kernel_sectors(FILE **image_file, Elf32_Ehdr *kernel_ehdr, Elf32_Phdr *kernel_phdr, int num_sec){
	fseek(*image_file, 0x80, SEEK_SET);
	fwrite(&num_sec, sizeof(int), 1, *image_file);
}

void record_sectors(FILE **image_file, int num_sec){
	fseek(*image_file, 0x80, SEEK_SET);
	fwrite(&num_sec, sizeof(int), 1, *image_file);
}


void extended_opt(Elf32_Phdr *boot_phdr, int k_phnum, Elf32_Phdr *kernel_phdr, int num_sec, FILE *boot_file, FILE *kernel_file){
	int i;
	long int b_length, k_length;
	fseek(boot_file, 0, SEEK_END);
	b_length = ftell(boot_file);
	fseek(kernel_file, 0, SEEK_END);
	k_length = ftell(kernel_file);
	printf("\nbootblock information:\n");
	printf("length of bootblock: %ld\n", b_length);
	printf("sectors: %d\n", num_sec);
	printf("offset of segment in the file(p_offset): 0x%x\n",boot_phdr->p_offset);
	printf("the virtual address of segment in memory(p_vaddr): 0x%x\n", boot_phdr->p_vaddr);
	printf("the file size of segment(p_filesz): 0x%x\n", boot_phdr->p_filesz);
	printf("the memory size of segment(p_memsz): 0x%x\n", boot_phdr->p_memsz);
	printf("\nkernel information:\n");
	printf("length of kernel: %ld\n", k_length);
	printf("sectors: %d\n", num_sec);
	for(i=0;i<k_phnum;i++) {
		printf("Segment: %d\n",i+1);
		printf("offset of segment in the file(p_offset): 0x%x\n",kernel_phdr[i].p_offset);
		printf("the virtual address of segment in memory(p_vaddr): 0x%x\n", kernel_phdr[i].p_vaddr);
		printf("the file size of segment(p_filesz): 0x%x\n", kernel_phdr[i].p_filesz);
		printf("the memory size of segment(p_memsz): 0x%x\n", kernel_phdr[i].p_memsz);
	}
}


int main(int argc, char *argv[]){
	FILE *boot_file;
	FILE *kernel_file;
	FILE *image_file;
	Elf32_Ehdr *kernel_ehdr;
	Elf32_Phdr *kernel_phdr;
	Elf32_Ehdr *boot_ehdr;
	Elf32_Phdr *boot_phdr;
	int num_sec;
	int k_phnum;

	FILE *process1_file;
	Elf32_Ehdr *process1_ehdr;
	Elf32_Phdr *process1_phdr;
	int process1_sector = 256;

	FILE *process2_file;
	Elf32_Ehdr *process2_ehdr;
	Elf32_Phdr *process2_phdr;
	int process2_sector = 272;

	FILE *process3_file;
	Elf32_Ehdr *process3_ehdr;
	Elf32_Phdr *process3_phdr;
	int process3_sector = 288;

	FILE *process4_file;
	Elf32_Ehdr *process4_ehdr;
	Elf32_Phdr *process4_phdr;
	int process4_sector = 304;

	kernel_ehdr = malloc(sizeof(Elf32_Ehdr));
	boot_ehdr = malloc(sizeof(Elf32_Ehdr));

	process1_ehdr = malloc(sizeof(Elf32_Ehdr));
	process2_ehdr = malloc(sizeof(Elf32_Ehdr));
	process3_ehdr = malloc(sizeof(Elf32_Ehdr));
	process4_ehdr = malloc(sizeof(Elf32_Ehdr));

	image_file = fopen("image", "w+");

	boot_phdr = read_exec_file(&boot_file, argv[argc-6], &boot_ehdr);
	write_bootblock(&image_file, boot_file, boot_ehdr, boot_phdr);

	kernel_phdr = read_exec_file(&kernel_file, argv[argc-5], &kernel_ehdr);
	write_kernel(&image_file, kernel_file, kernel_ehdr, kernel_phdr);

	process1_phdr = read_exec_file(&process1_file, argv[argc-4], &process1_ehdr);
	write_process(&image_file, process1_file, process1_ehdr, process1_phdr, process1_sector);
	process2_phdr = read_exec_file(&process2_file, argv[argc-3], &process2_ehdr);
	write_process(&image_file, process2_file, process2_ehdr, process2_phdr, process2_sector);
	process3_phdr = read_exec_file(&process3_file, argv[argc-2], &process3_ehdr);
	write_process(&image_file, process3_file, process3_ehdr, process3_phdr, process3_sector);
	process4_phdr = read_exec_file(&process4_file, argv[argc-1], &process4_ehdr);
	write_process(&image_file, process4_file, process4_ehdr, process4_phdr, process4_sector);

	k_phnum = kernel_ehdr->e_phnum;

//	num_sec = count_kernel_sectors(kernel_ehdr, kernel_phdr);
//	record_kernel_sectors(&image_file, kernel_ehdr, kernel_phdr, num_sec);
	num_sec = process4_sector + count_sectors(process4_ehdr, process4_phdr) - 1;
	record_sectors(&image_file, num_sec);

	if(!strncmp("--extended", argv[1], 11)) extended_opt(boot_phdr, k_phnum, kernel_phdr, num_sec, boot_file, kernel_file);

	fclose(boot_file);
	fclose(kernel_file);
	fclose(image_file);
	fclose(process1_file);
	fclose(process2_file);
	fclose(process3_file);
	fclose(process4_file);

	free(kernel_ehdr);
	free(kernel_phdr);
	free(boot_ehdr);
	free(boot_phdr);
	free(process1_ehdr);
	free(process1_phdr);
	free(process2_ehdr);
	free(process2_phdr);
	free(process3_ehdr);
	free(process3_phdr);
	free(process4_ehdr);
	free(process4_phdr);
}
