/*
 * Methods copied from /sys/arch/arm64/stand/efiboot/fdt.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "fdt.h"

extern struct fdt fdt;

//#define DEBUG

/*
 * Retrieve string pointer from strings table.
 */
char *
fdt_get_str(uint32_t num)
{
	if (num > fdt.strings_size)
		return NULL;
	return (fdt.strings) ? (fdt.strings + num) : NULL;
}
		
char *
fdt_node_name(void *node)
{
	uint32_t *ptr;

#ifdef DEBUG
	printf("//fdt.c:fdt_node_name:\n");
	printf("//node=0x%x\n", node);
#endif

	//if (!tree_inited)
	//	return NULL;

	ptr = node;

	if (betoh32(*ptr) != FDT_NODE_BEGIN)
		return NULL;

	if (*(ptr+1) == 0x0)
			ptr += 4;

	return (char *)(ptr + 1);
}

/*
 * Utility functions for skipping parts of tree.
 */
void *
skip_property(uint32_t *ptr)
{
	uint32_t size;

#ifdef DEBUG
	printf("//fdt.c:skip_property:\n");
#endif

	size = betoh32(*(ptr + 1));
	/* move forward by magic + size + nameid + rounded up property size */
	ptr += 3 + roundup(size, sizeof(uint32_t)) / sizeof(uint32_t);

	return ptr;
}

void *
skip_props(uint32_t *ptr)
{
#ifdef DEBUG
	printf("//fdt.c:skip_props:\n");
#endif
	while (betoh32(*ptr) == FDT_PROPERTY) {
		ptr = skip_property(ptr);
	}
	return ptr;
}

void *
skip_node_name(uint32_t *ptr)
{
#ifdef DEBUG
	printf("//fdt.c:skip_node_name:\n");
#endif
	/* skip name, aligned to 4 bytes, this is NULL term., so must add 1 */
	return ptr + roundup(strlen((char *)ptr) + 1,
	    sizeof(uint32_t)) / sizeof(uint32_t);
}

/*
 * Retrieves next node, skipping all the children nodes of the pointed node,
 * returns pointer to next node, no matter if it exists or not.
 */
void *
skip_node(void *node)
{
	uint32_t *ptr = node;

#ifdef DEBUG
	printf("//fdt.c:skip_node:\n");
#endif
	ptr++;

	ptr = skip_node_name(ptr);
	ptr = skip_props(ptr);

	/* skip children */
	while (betoh32(*ptr) == FDT_NODE_BEGIN)
		ptr = skip_node(ptr);

	return (ptr + 1);
}


/*
 * Retrieves next node, skipping all the children nodes of the pointed node
 */
void *
fdt_child_node(void *node)
{
	uint32_t *ptr;

#ifdef DEBUG
	printf("//fdt.c:fdt_child_node:\n");
#endif
//	if (!tree_inited)
//		return NULL;

	ptr = node;

	if (betoh32(*ptr) != FDT_NODE_BEGIN)
		return NULL;

	ptr++;

	ptr = skip_node_name(ptr);
	ptr = skip_props(ptr);
	/* check if there is a child node */
	return (betoh32(*ptr) == FDT_NODE_BEGIN) ? (ptr) : NULL;
}
	 	
/*
 * Retrieves next node, skipping all the children nodes of the pointed node,
 * returns pointer to next node if exists, otherwise returns NULL.
 * If passed 0 will return first node of the tree (root).
 */
void *
fdt_next_node(void *node)
{
	uint32_t *ptr;
#ifdef DEBUG
	printf("//fdt.c:fdt_next_node:\n");
#endif

//	if (!tree_inited)
//		return NULL;

	ptr = node;

//	if (node == NULL) {
//		ptr = (uint32_t *)tree.tree;
//		return (betoh32(*ptr) == FDT_NODE_BEGIN) ? ptr : NULL;
//	}

	if (betoh32(*ptr) != FDT_NODE_BEGIN)
		return NULL;

	ptr++;

	ptr = skip_node_name(ptr);
	ptr = skip_props(ptr);

	/* skip children */
	while (betoh32(*ptr) == FDT_NODE_BEGIN)
		ptr = skip_node(ptr);

	if (betoh32(*ptr) != FDT_NODE_END)
		return NULL;

	if (betoh32(*(ptr + 1)) != FDT_NODE_BEGIN)
		return NULL;

	return (ptr + 1);
}

/*
 * Debug methods for printing whole tree, particular nodes and properties
 */
void *
fdt_print_property(void *node, int level)
{
	uint32_t *ptr;
	char *tmp, *value;
	int cnt;
	uint32_t nameid, size;

#ifdef DEBUG
	printf("//fdt.c:fdt_print_property:\n");
#endif
	ptr = (uint32_t *)node;

//	if (!tree_inited)
//		return NULL;

	if (betoh32(*ptr) != FDT_PROPERTY)
		return ptr; /* should never happen */

	/* extract property name_id and size */
	size = betoh32(*++ptr);
	nameid = betoh32(*++ptr);

	for (cnt = 0; cnt < level; cnt++)
		printf("\t");

	tmp = fdt_get_str(nameid);
	printf("\t%s : ", tmp ? tmp : "NO_NAME");

	ptr++;
	value = (char *)ptr;

	if (!strcmp(tmp, "device_type") || !strcmp(tmp, "compatible") ||
	    !strcmp(tmp, "model") || !strcmp(tmp, "bootargs") ||
	    !strcmp(tmp, "linux,stdout-path")) {
		printf("%s", value);
	} else if (!strcmp(tmp, "stdout-path") || !strcmp(tmp, "pinctrl-names") ||
		!strcmp(tmp, "regulator-name") || !strcmp(tmp, "charger-type") ||
		!strcmp(tmp, "label") || !strcmp(tmp, "function") ||
		!strcmp(tmp, "clock-names") || !strcmp(tmp, "reset-names") ||
		!strcmp(tmp, "status") || !strcmp(tmp, "description") ||
		!strcmp(tmp, "os") || !strcmp(tmp, "arch") ||
		!strcmp(tmp, "type") || !strcmp(tmp, "compression") ||
		!strcmp(tmp, "firmware") || !strcmp(tmp, "default") ||
		!strcmp(tmp, "loadables") || !strcmp(tmp, "fdt") ||
		!strcmp(tmp, "enable-method") || !strcmp(tmp, "method") ||
		!strncmp(tmp, "phy", 3) || !strcmp(tmp, "interrupt-names") ||
		!strncmp(tmp, "ethernet", 8) || !strncmp(tmp, "serial", 6) ||
		!strncmp(tmp, "spi", 3) || !strncmp(tmp, "u-boot", 6) ||
		!strncmp(tmp, "i2c", 3) || !strncmp(tmp, "mmc", 3)) {
		printf("%s", value);
	} else if (!strcmp(tmp, "clock-frequency") ||
	    !strcmp(tmp, "timebase-frequency")) {
		printf("%d", betoh32(*((unsigned int *)value)));
	} else {
		for (cnt = 0; cnt < size; cnt++) {
			if ((cnt % sizeof(uint32_t)) == 0)
				printf(" ");
			printf("%x%x", value[cnt] >> 4, value[cnt] & 0xf);
		}
	}
	ptr += roundup(size, sizeof(uint32_t)) / sizeof(uint32_t);
	printf("\n");

	return ptr;
}

void
fdt_print_node(void *node, int level)
{
	uint32_t *ptr;
	int cnt;
	
#ifdef DEBUG
	printf("//fdt.c:fdt_print_node:\n");
#endif
	ptr = (uint32_t *)node;

	if (betoh32(*ptr) != FDT_NODE_BEGIN)
		return;

	ptr++;

	for (cnt = 0; cnt < level; cnt++)
		printf("\t");
	printf("%s :\n", fdt_node_name(node));
	ptr = skip_node_name(ptr);

	while (betoh32(*ptr) == FDT_PROPERTY)
		ptr = fdt_print_property(ptr, level);
}

void
fdt_print_node_recurse(void *node, int level)
{
	void *child;
#ifdef DEBUG
	printf("//fdt.c:fdt_print_node_recurse:\n");
#endif

	fdt_print_node(node, level);
	for (child = fdt_child_node(node); child; child = fdt_next_node(child))
		fdt_print_node_recurse(child, level + 1);
}
