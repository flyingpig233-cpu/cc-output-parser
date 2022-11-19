#ifndef _OUTPUT_PARSER_H__
#define _OUTPUT_PARSER_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif //_GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONST_STR_LEN(str) (sizeof(str) - 1)
#define ERROR_MATCH "error:"
#define WARNING_MATCH "warning:"
#define NOTE_MATCH "note:"

enum output_type {
    Info,
    Warning,
    Error,
    Note
};

struct output_list {
    enum output_type type;
    char* message;
    char* filename;
    int line;
    int col;
    struct output_list* next;
};

static inline const char* get_type_name(enum output_type type)
{
    switch (type) {
    case Info:
        return "Info";
    case Warning:
        return "Warning";
    case Error:
        return "Error";
    case Note:
        return "Note";
    default:
        return "Unkown";
    }
}

int parse_one_line(const char* line, size_t len, struct output_list* data)
{
    char* pos;

    if ((pos = memmem(line, len, "|", 1)) != NULL) {
        return -1;
    }

    if ((pos = memmem(line, len, ":", 1)) == NULL) {
        return -1;
    }

    char* lpos = ++pos;
    // match message type
    if ((pos = memmem(line, len, ERROR_MATCH, CONST_STR_LEN(ERROR_MATCH))) != NULL) {
        data->type = Error;
    } else if ((pos = memmem(line, len, WARNING_MATCH, CONST_STR_LEN(WARNING_MATCH))) != NULL) {
        data->type = Warning;
    } else if ((pos = memmem(line, len, NOTE_MATCH, CONST_STR_LEN(NOTE_MATCH))) != NULL) {
        data->type = Note;
    } else {
        data->type = Info;
        pos = lpos;
    }

    // get filename
    size_t filename_len = lpos - line - 1;
    char* filename = (char*)malloc(filename_len + 1);
    memcpy(filename, line, filename_len);
    filename[filename_len] = '\0';
    data->filename = filename;

    // get message
    size_t msg_len = len - (pos - line);
    char* msg = (char*)malloc(msg_len + 1);
    strncpy(msg, pos, msg_len);
    data->message = msg;

    if (lpos != pos) {
        if (sscanf(lpos, "%d:%d", &data->line, &data->col) != 2) {
            assert("Match failed!");
        }
    }
    return 0;
}

struct output_list* parse_cc_output(char* output)
{
    size_t output_len = strlen(output);
    struct output_list* head = (struct output_list*)malloc(sizeof(struct output_list));
    struct output_list* node = head;
    char* line_begin = output;
    int ret = 0;
    for (;;) {
        char* line_end = strchr(line_begin, '\n');
        if (line_end == NULL) {
            line_end = output + output_len + 1;
            ret = 1;
        }
        if (!parse_one_line(line_begin, line_end - line_begin, node)) {
            struct output_list* list = node;
            // printf("type: %s\nmessage: %s\nfilename: %s\nline: %d\tcol: %d\n",
            //     get_type_name(list->type), list->message, list->filename,
            //     list->line, list->col);
            node->next = (struct output_list*)malloc(sizeof(struct output_list));
            node = node->next;
        }
        if (ret)
            break;
        line_begin = line_end + 1;
    }
    return head;
}

static inline void drop_output_list_elements(struct output_list* data)
{
    free(data->message);
    free(data->filename);
}

void drop_output_list(struct output_list* head)
{
    struct output_list* next;
    while (head != NULL) {
        next = head->next;
        drop_output_list_elements(head);
        free(head);
        head = next;
    }
}

#endif // _OUTPUT_PARSER_H__
