#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IMG_REX "Pasted image [0-9]{14}.png"
#define IMG_MAX 1024

bool is_markdown(const char *file);
int cp(const char *to, const char *from);

int main(int argc, char *argv[]) {
  // File variables
  FILE *fp;
  char *file_line = NULL;
  size_t line_len = 0;
  ssize_t read;
  char src_file_path[IMG_MAX][1024];
  char dest_file_path[IMG_MAX][1024];

  // Parsed image variables
  char img_list[IMG_MAX][50];
  int img_num = 0;

  // Regex variables
  regex_t re;
  regmatch_t rm[2];
  int retval = 0;
  regmatch_t pmatch[2] = {{-1, -1}, {-1, -1}};

  if (argc != 5) {
    printf("Usage:\n%s [obsidian markdown file] [obsidian image directory] "
           "[destination directory] [desired name]\n\n",
           argv[0]);
    printf(
        "[obsidian markdown file]:              the .md file with references "
        "to images you'd like to extract \n"
        "                                       (expected format \"![[Pasted "
        "image timestamp.png]])\"\n");
    printf("[obsidian image directory]:            the directory that contains "
           "your "
           "obsidian images\n");
    printf(
        "[destination directory]:               the directory you wish to save "
        "your "
        "files to\n");
    printf("[desired name]:                        name of the final file (no "
           "spaces)\n");
    exit(EXIT_FAILURE);
  }

  printf("Sudmitted arguments:\nObsidian markdown file to be parsed-%s\n",
         argv[1]);
  printf("Obsidian image directory-%s\n", argv[2]);
  printf("Destination directory-%s\n", argv[3]);
  printf("Desired file name-%s\n\n", argv[4]);

  if (!(is_markdown(argv[1]))) {
    printf("No markdown file extension detected.\n");
    exit(EXIT_FAILURE);
  }

  if ((fp = fopen(argv[1], "r")) == NULL) {
    printf("Could not open file %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  // Regex compile
  if (regcomp(&re, IMG_REX, REG_EXTENDED) != 0) {
    fprintf(stderr, "Failed to compile regex '%s'\n", IMG_REX);
    exit(EXIT_FAILURE);
  }

  // Read file line by line
  while ((read = getline(&file_line, &line_len, fp)) != -1) {
    retval = regexec(&re, file_line, 2, pmatch, 0);
    if (!retval) {
      /*
      printf("Line: %s", file_line);
      printf("match offsets are %d %d\n", pmatch[0].rm_so, pmatch[0].rm_eo);
      printf("match[0]=%*s", pmatch[0].rm_eo, &file_line[pmatch[0].rm_so]);
      */
      file_line[strlen(file_line) - 3] = '\0';
      // printf("file line-%s\n", file_line + 3);
      strcpy(img_list[img_num], file_line + 3);
      // printf("img_list-%s\n", &img_list[img_num]);
      img_num++;
    } else {
      printf("No screenshot names matches in %s with regex pattern-\"%s\"\n",
             argv[1], IMG_REX);
      exit(EXIT_FAILURE);
    }
  }

  for (int n = 0; n < img_num; n++) {
    // printf("Found screenshot-%s\n", img_list[n]);
    strcpy(src_file_path[n], argv[2]);
    strcat(src_file_path[n], img_list[n]);
    strcpy(dest_file_path[n], argv[3]);
    strcat(dest_file_path[n], argv[4]);
    snprintf(dest_file_path[n] + strlen(dest_file_path[n]), IMG_MAX, "%d",
             n + 1);
    strcat(dest_file_path[n], ".png");
    printf("Attempting to copy %s to %s\n", src_file_path[n],
           dest_file_path[n]);
    if ((cp(dest_file_path[n], src_file_path[n])) != 0) {
      printf("Failed to copy %s to %s\n", src_file_path[n], dest_file_path[n]);
    } else {
      printf("Success!\n");
    }
  }

  if ((fclose(fp)) != false) {
    printf("Could not close file %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  if (file_line) {
    free(file_line);
  }

  return 0;
}

bool is_markdown(const char *file) {
  char *ext = strchr(file, '.');
  if (ext && strcmp(ext, "md")) {
    return true;
  } else {
    return false;
  }
}

int cp(const char *to, const char *from) {
  int fd_to, fd_from;
  char buf[4096];
  ssize_t nread;
  int saved_errno;

  fd_from = open(from, O_RDONLY);
  if (fd_from < 0) {
    printf("Failed ot open %s\n", from);
    return -1;
  }

  fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0) {
    printf("Failed ot open %s\n", to);
    goto out_error;
  }

  while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
    char *out_ptr = buf;
    ssize_t nwritten;

    do {
      nwritten = write(fd_to, out_ptr, nread);

      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if (errno != EINTR) {
        goto out_error;
      }
    } while (nread > 0);
  }

  if (nread == 0) {
    if (close(fd_to) < 0) {
      fd_to = -1;
      goto out_error;
    }
    close(fd_from);

    /* Success! */
    return 0;
  }

out_error:
  saved_errno = errno;

  close(fd_from);
  if (fd_to >= 0)
    close(fd_to);

  errno = saved_errno;
  return -1;
}
