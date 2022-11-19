#include "include/parser.h"

int main(void)
{
    FILE* read_fp;
    char buffer[BUFSIZ + 1] = { 0 };
    int chars_read;
	printf("Enter the file to build: ");
	fflush(0);
	char filename[201];
	if (scanf("%200s", filename) != 1) {
		return EXIT_FAILURE;
	}
	char command[301];
	sprintf(command, "gcc %s 2>&1", filename);
    printf("Running command: %s\n", command);
    read_fp = popen(command, "r");
    if (read_fp != NULL) {
        chars_read = fread(buffer, sizeof(char), BUFSIZ, read_fp);
        if (chars_read > 0) {
            struct output_list* list = parse_cc_output(buffer);
			drop_output_list(list);
			printf("Success to drop output lists\n");

        }
        pclose(read_fp);
        return 0;
    }
    return -1;
}

