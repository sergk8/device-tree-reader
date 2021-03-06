#include <endian.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>


#include "fdt.h"

#define DEFAULT_DTB_FILE "/usr/local/share/dtb/arm64/rockchip/rk3399-pinebook-pro.dtb";

//#define DEBUG
//#define PRINT_STRINGS
//#define PRINT_NODE_DUMP
struct fdt fdt;


/* flags */
static int f_strings;
static int f_dump;
static int f_tree;
static int f_tree_default;


static int
find_fdt_signature(void * const mem, int size, const void ** fdt_ptr) {
	char * ptr;
	int offset = 0;

	ptr = (char *) mem;

#ifdef DEBUG
	printf("find_fdt_signature\n");
#endif

	while (!(*(ptr + 0) == 0xd0 && *(ptr + 1) == 0x0d && *(ptr + 2) == 0xfe && *(ptr + 3) == 0xed)) {
		offset++;
		if (offset > size - 5) {
			return -1;
		}
		ptr++;
	}
	printf("Found FDT_MAGIC at offset: %d\n", offset);

	*fdt_ptr = (void *)ptr;
	return 1;
}


static void
usage() {
		printf("Usage: %s <option(s)> <file>\n", "./main");
		printf("One of the following switches can be given:\n");
		printf("\
\t-s	Display strings (and their offsets) from dtb  file\n\
\t-d	Display dump of dtb file\n\
\t-t	Display FDT tree (default behaviour)\n\
");
}

static void
show_strings() {
		uint32_t c;

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
		}
}

static void
show_dump(uint32_t *ptr) {
		uint32_t c;

		for (c = 0; c <= 256; c += sizeof(char)) {
			char elem = *((char *)ptr + c);
			printf("[0x%x] fdt.tree+%d: %c (%d)", (char *)ptr + c, c, elem, elem);
			switch (elem) {
					case FDT_NODE_BEGIN:
						printf(" <== FDT_NODE_BEGIN");
						break;
					case FDT_NODE_END:
						printf(" <== FDT_NODE_END");
						break;
					case FDT_PROPERTY:
						printf(" <== FDT_PROPERTY");
						break;
					case FDT_NOP:
						printf(" <== FDT_NOP");
						break;
					case FDT_END:
						printf(" <== FDT_END");
						break;
					default:
						break;
			}
			printf("\n");
		}
		//printf("\n\n//fdt.tree=0x%x\n", fdt.tree);
}

int
main(int argc, char *argv[]) {
		char *dtb_file;
		uint32_t *addr, c = 0, *ptr, *fdt_ptr;
		int fd;
		struct stat sb;

		struct fdt_head *header;

		int ch;


		f_tree_default = 1;
		while((ch = getopt(argc, argv, "sdt")) != -1) {
			switch(ch) {
					case 's':
							f_strings = 1;
							f_tree_default = 0;
							break;
					case 'd':
							f_dump = 1;
							f_tree_default = 0;
							break;
					case 't':
							f_tree = 1;
							break;
					default:
							break;
			}
		}
		argc -= optind;
		argv += optind;

		if (argc < 1) {
				usage();
				return 0;
		}

		dtb_file = argv[--argc];

		printf("opening dtb file: %s\n", dtb_file);

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

		if (find_fdt_signature(addr, sb.st_size, &fdt_ptr) < 0) {
			printf("Could not find FDT_MAGIC\n");
			return 0;
		}

		//fdt = (struct fdt*)addr;
		fdt.header = (struct fdt_head*)fdt_ptr;
		fdt.tree = (char *)fdt_ptr + htobe32(fdt.header->fh_struct_off);
		fdt.strings = (char *)fdt_ptr + htobe32(fdt.header->fh_strings_off);
		fdt.memory = (char *)fdt_ptr + htobe32(fdt.header->fh_reserve_off);
		fdt.end = (char *)fdt_ptr + htobe32(fdt.header->fh_size);
		fdt.strings_size = htobe32(fdt.header->fh_strings_size);
		fdt.struct_size = htobe32(fdt.header->fh_struct_size);


//		printf("reading bytes: \n");
//		for (c = 0; c <= sb.st_size; c += sizeof(uint32_t)) {
			//printf("%d: %x\n", c, htobe32(*(addr+c)));
			//printf("%x", htobe32(*(addr+c)));
//		}

//		printf("%x\n", fdt->header);
//		printf("magic: 0x%x\n", htobe32(fdt.header->fh_magic));
		printf("version: %x\n", htobe32(fdt.header->fh_version));
		printf("strings_size: %d\n", fdt.strings_size);
		printf("struct_size: %d\n", fdt.struct_size);


		if (f_strings == 1) {
				printf("\nDumping strings:\n\n");
				show_strings();
		}

		ptr = (uint64_t *)fdt.tree;
		if (f_dump == 1) { 
				printf("\nDumping binary:\n\n");
				show_dump(ptr);
		}

		if (f_tree == 1 || f_tree_default == 1) {
				printf("\nDumping tree:\n\n");
				fdt_print_node_recurse(ptr, 0);
		}
		return 1;
}

