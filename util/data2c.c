#include <stdio.h>
#include <stdlib.h>

void usage(const char *progname) {
	fprintf(stderr, "Usage: %s <archivename> [input files...]\n", progname);
	exit(1);
}

void dump_to_file(FILE *ofp, char *fname) {
	FILE *ifp = fopen(fname, "rb");
	if (!ifp) {
		printf("Unable to open file '%s'\n", fname);
		fclose(ofp);
		exit(1);
	}

	size_t fsize = 0;
	int c;

	fprintf(ofp, "{\"%s\",(unsigned char[]){", fname);

	while ((c = fgetc(ifp)) != EOF) {
		fprintf(ofp, "0x%x,", c);
		fsize++;
	}

	fprintf(ofp, "},%zu,},\n", fsize);

	fclose(ifp);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		usage(argv[0]);
	}

	FILE *ofp = fopen(argv[1], "w");
	if (!ofp) {
		usage(argv[0]);
	}

	fputs("// This file has been automatically generated.  Do not edit.\n", ofp);
	fputs("struct {\n"
	      "const char *name;\n"
	      "const unsigned char *data;\n"
	      "size_t size;\n"
	      "} dlbembed_data[] = {\n",
	      ofp);

	for (int i = 2; i < argc; i++) {
		dump_to_file(ofp, argv[i]);
	}

	fputs("};\n", ofp);

	fclose(ofp);
}
