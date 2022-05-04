#include <endian.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>


#include "fdt.h"

int main() {
		char *dtb_file = "/usr/local/share/dtb/arm64/rockchip/rk3399-pinebook-pro.dtb";

		uint32_t *addr, c = 0;
		int fd;
		struct stat sb;

		struct fdt fdt;
		struct fdt_head *header;

		printf("opening dtb file\n");

		fd = open(dtb_file, O_RDONLY);
		if (fd < 0 || fstat(fd, &sb) == -1) {
			printf("cannot open %s\n", dtb_file);
			return 0;
		}

		addr = (uint32_t *) malloc(sb.st_size * sizeof(uint32_t));

		printf("dtb file size: %lld\n", sb.st_size);
		if (read(fd, addr, sb.st_size) != sb.st_size) {
			printf("cannot read from %s\n", dtb_file);
			return 0;
		}

		//fdt = (struct fdt*)addr;
		fdt.header = (struct fdt_head*)addr;
		fdt.tree = (char *)addr + htobe32(fdt.header->fh_struct_off);
		fdt.strings = (char *)addr + htobe32(fdt.header->fh_strings_off);
		fdt.memory = (char *)addr + htobe32(fdt.header->fh_reserve_off);
		fdt.end = (char *)addr + htobe32(fdt.header->fh_size);
		fdt.strings_size = htobe32(fdt.header->fh_strings_size);


		printf("reading bytes: \n");
		for (c = 0; c <= sb.st_size; c += sizeof(uint32_t)) {
			//printf("%d: %x\n", c, htobe32(*(addr+c)));
			//printf("%x", htobe32(*(addr+c)));
		}

//		printf("%x\n", fdt->header);
		printf("magic: %x\n", htobe32(fdt.header->fh_magic));
		printf("version: %x\n", htobe32(fdt.header->fh_version));
		printf("strings_size: %x\n", htobe32(fdt.strings_size));

		printf("\nDumping tree:\n\n");


		printf("Printing all strings:\n");
		for (c = 0; c <= fdt.strings_size; c += sizeof(char)) {
			char x = *(fdt.strings + c);

			if ( c== 0 || *(fdt.strings + c - sizeof(char)) == '\0') {

				printf("%d: ", c);
			}

			if (x == '\0') {
				printf("\n");
			} else {
				printf("%c", (x));
			}
			//printf("%x", htobe32(*(addr+c)));
		}
		return 1;
}

