/*
  Copyright (c) 2022 William Cotton

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "string.h"
#include <Block.h>

static int compare_strings(const void *a, const void *b) {
  string_t *s1 = *(string_t **)a;
  string_t *s2 = *(string_t **)b;
  return strcmp(s1->value, s2->value);
}

char *stringErrorMessage(int error) {
  switch (error) {
  case 0:
    return NULL;
  case NUMBER_ERROR_NO_DIGITS:
    return "invalid (no digits found, 0 returned)";
  case NUMBER_ERROR_UNDERFLOW:
    return "invalid (underflow occurred)";
  case NUMBER_ERROR_OVERFLOW:
    return "invalid (overflow occurred)";
  case NUMBER_ERROR_BASE_UNSUPPORTED:
    return "invalid (base contains unsupported value)";
  case NUMBER_ERROR_UNSPECIFIED:
    return "invalid (unspecified error occurred)";
  case NUMBER_ERROR_ADDITIONAL_CHARACTERS:
    return "valid (additional characters found)";
  default:
    return "invalid (unspecified error occurred)";
  }
}

string_collection_t *stringCollection(size_t size, string_t **array) {
  string_collection_t *collection = malloc(sizeof(string_collection_t));
  collection->size = size;
  collection->arr = array;

  collection->mallocCount = 0;
  collection->malloc = Block_copy(^(size_t msize) {
    void *ptr = malloc(msize);
    collection->mallocs[collection->mallocCount++] = (malloc_t){.ptr = ptr};
    return ptr;
  });

  collection->blockCopyCount = 0;
  collection->blockCopy = Block_copy(^(void *block) {
    void *ptr = Block_copy(block);
    collection->blockCopies[collection->blockCopyCount++] =
        (malloc_t){.ptr = ptr};
    return ptr;
  });

  collection->each = collection->blockCopy(^(eachStringCallback callback) {
    for (size_t i = 0; i < collection->size; i++) {
      callback(collection->arr[i]);
    }
  });

  collection->eachWithIndex =
      collection->blockCopy(^(eachStringWithIndexCallback callback) {
        for (size_t i = 0; i < collection->size; i++) {
          callback(collection->arr[i], i);
        }
      });

  collection->reduce = collection->blockCopy(
      ^(void *accumulator, reducerStringCallback reducer) {
        for (size_t i = 0; i < collection->size; i++) {
          accumulator = reducer(accumulator, collection->arr[i]);
        }
        return accumulator;
      });

  collection->map = collection->blockCopy(^(mapStringCallback callback) {
    void **arr = collection->malloc(sizeof(void *) * collection->size);
    for (size_t i = 0; i < collection->size; i++) {
      arr[i] = callback(collection->arr[i]);
    }
    return arr;
  });

  collection->reverse = collection->blockCopy(^(void) {
    for (size_t i = 0; i < collection->size / 2; i++) {
      string_t *tmp = collection->arr[i];
      collection->arr[i] = collection->arr[collection->size - i - 1];
      collection->arr[collection->size - i - 1] = tmp;
    }
    return collection;
  });

  collection->push = collection->blockCopy(^(string_t *string) {
    collection->size++;
    collection->arr =
        realloc(collection->arr, collection->size * sizeof(string_t *));
    collection->arr[collection->size - 1] = string;
    return collection;
  });

  collection->sort = collection->blockCopy(^{
    // sort strings with qsort
    qsort(collection->arr, collection->size, sizeof(string_t *),
          compare_strings);
    return collection;
  });

  collection->join = collection->blockCopy(^(const char *delim) {
    size_t delimSize = strlen(delim);
    if (collection->size == 0) {
      return string("");
    }
    size_t len = 0;
    for (size_t i = 0; i < collection->size; i++) {
      len += collection->arr[i]->size;
    }
    len += collection->size - 1;
    char *str = malloc(len + 1);
    str[0] = '\0';
    for (size_t i = 0; i < collection->size; i++) {
      strlcat(str, collection->arr[i]->value,
              len + 1 + collection->arr[i]->size);
      if (i < collection->size - 1) {
        strlcat(str, delim, len + 1 + delimSize);
      }
    }
    string_t *temp = string(str);
    free(str);
    return temp;
  });

  collection->indexOf = collection->blockCopy(^(const char *str) {
    for (int i = 0; i < (int)collection->size; i++) {
      if (strcmp(collection->arr[i]->value, str) == 0) {
        return i;
      }
    }
    return -1;
  });

  collection->first = collection->blockCopy(^{
    return collection->arr[0];
  });
  collection->second = collection->blockCopy(^{
    return collection->arr[1];
  });
  collection->third = collection->blockCopy(^{
    return collection->arr[2];
  });
  collection->fourth = collection->blockCopy(^{
    return collection->arr[3];
  });
  collection->fifth = collection->blockCopy(^{
    return collection->arr[4];
  });
  collection->last = collection->blockCopy(^{
    return collection->arr[collection->size - 1];
  });

  collection->free = Block_copy(^(void) {
    for (size_t i = 0; i < collection->size; i++) {
      collection->arr[i]->free();
    }
    free(collection->arr);

    for (int i = 0; i < collection->mallocCount; i++) {
      free(collection->mallocs[i].ptr);
    }
    for (int i = 0; i < collection->blockCopyCount; i++) {
      Block_release(collection->blockCopies[i].ptr);
    }
    Block_release(collection->blockCopy);

    dispatch_async(dispatch_get_main_queue(), ^() {
      Block_release(collection->free);
      free(collection);
    });
  });

  return collection;
}

string_t *string(const char *strng) {
  string_t *s = malloc(sizeof(string_t));
  s->value = strdup(strng);
  s->size = strlen(s->value);

  s->blockCopyCount = 0;
  s->blockCopy = Block_copy(^(void *block) {
    void *ptr = Block_copy(block);
    s->blockCopies[s->blockCopyCount++] = (malloc_t){.ptr = ptr};
    return ptr;
  });

  s->print = s->blockCopy(^(void) {
    printf("%s\n", s->value);
  });

  s->concat = s->blockCopy(^(const char *str) {
    size_t size = s->size + strlen(str);
    char *new_str = malloc(size + 1);
    strlcpy(new_str, s->value, size + 1);
    strlcat(new_str, str, size + 1);
    free(s->value);
    s->value = new_str;
    s->size = size;
    return s;
  });

  s->upcase = s->blockCopy(^(void) {
    for (size_t i = 0; i < s->size; i++) {
      s->value[i] = toupper(s->value[i]);
    }
    return s;
  });

  s->downcase = s->blockCopy(^(void) {
    for (size_t i = 0; i < s->size; i++) {
      s->value[i] = tolower(s->value[i]);
    }
    return s;
  });

  s->capitalize = s->blockCopy(^(void) {
    for (size_t i = 0; i < s->size; i++) {
      if (i == 0) {
        s->value[i] = toupper(s->value[i]);
      } else {
        s->value[i] = tolower(s->value[i]);
      }
    }
    return s;
  });

  s->reverse = s->blockCopy(^(void) {
    char *new_str = malloc(s->size + 1);
    for (size_t i = 0; i < s->size; i++) {
      new_str[s->size - i - 1] = s->value[i];
    }
    new_str[s->size] = '\0';
    free(s->value);
    s->value = new_str;
    return s;
  });

  s->trim = s->blockCopy(^(void) {
    size_t start = 0;
    size_t end = s->size - 1;
    while (isspace(s->value[start])) {
      start++;
    }
    while (isspace(s->value[end])) {
      end--;
    }
    size_t size = end - start + 1;
    char *new_str = malloc(size + 1);
    strncpy(new_str, s->value + start, size);
    new_str[size] = '\0';
    free(s->value);
    s->value = new_str;
    s->size = size;
    return s;
  });

  s->split = s->blockCopy(^(const char *delim) {
    string_collection_t *collection = stringCollection(0, NULL);
    collection->size = 0;
    collection->arr = malloc(sizeof(string_t *));
    char *tknPtr;
    char *token = strtok_r(s->value, delim, &tknPtr);
    while (token != NULL) {
      collection->size++;
      collection->arr =
          realloc(collection->arr, collection->size * sizeof(string_t *));
      collection->arr[collection->size - 1] = string(token);
      token = strtok_r(NULL, delim, &tknPtr);
    }
    return collection;
  });

  s->toInt = s->blockCopy(^(void) {
    char *nptr = s->value;
    char *endptr = NULL;
    int error = 0;
    long long number;
    errno = 0;
    number = strtoll(nptr, &endptr, 10);
    if (nptr == endptr) {
      error = NUMBER_ERROR_NO_DIGITS;
    } else if (errno == ERANGE && number == LLONG_MIN) {
      error = NUMBER_ERROR_UNDERFLOW;
    } else if (errno == ERANGE && number == LLONG_MAX) {
      error = NUMBER_ERROR_OVERFLOW;
    } else if (errno == EINVAL) { /* not in all c99 implementations - gcc OK */
      error = NUMBER_ERROR_BASE_UNSUPPORTED;
    } else if (errno != 0 && number == 0) {
      error = NUMBER_ERROR_UNSPECIFIED;
    } else if (errno == 0 && nptr && *endptr != 0) {
      error = NUMBER_ERROR_ADDITIONAL_CHARACTERS;
    }
    integer_number_t n;
    if (error == 0 || error == NUMBER_ERROR_ADDITIONAL_CHARACTERS) {
      n.value = number;
    } else {
      n.value = 0;
    }
    n.error = error;
    return n;
  });

  s->toDecimal = s->blockCopy(^(void) {
    char *nptr = s->value;
    char *endptr = NULL;
    int error = 0;
    long double number;
    errno = 0;
    number = strtold(nptr, &endptr);
    if (nptr == endptr) {
      error = NUMBER_ERROR_NO_DIGITS;
    } else if (errno == ERANGE && number == -HUGE_VALL) {
      error = NUMBER_ERROR_UNDERFLOW;
    } else if (errno == ERANGE && number == HUGE_VALL) {
      error = NUMBER_ERROR_OVERFLOW;
    } else if (errno == EINVAL) { /* not in all c99 implementations - gcc OK */
      error = NUMBER_ERROR_BASE_UNSUPPORTED;
    } else if (errno != 0 && number == 0) {
      error = NUMBER_ERROR_UNSPECIFIED;
    } else if (errno == 0 && nptr && *endptr != 0) {
      error = NUMBER_ERROR_ADDITIONAL_CHARACTERS;
    }
    decimal_number_t n;
    if (error == 0 || error == NUMBER_ERROR_ADDITIONAL_CHARACTERS) {
      n.value = number;
    } else {
      n.value = 0;
    }
    n.error = error;
    return n;
  });

  s->replace = s->blockCopy(^(const char *str1, const char *str2) {
    size_t str1_len = strlen(str1);
    size_t str2_len = strlen(str2);
    size_t newStrLen = s->size + str2_len - str1_len + 1;
    char *newStr = malloc(newStrLen);
    size_t i = 0;
    size_t j = 0;
    while (i < s->size) {
      if (strncmp(s->value + i, str1, strlen(str1)) == 0) {
        strlcpy(newStr + j, str2, newStrLen);
        i += strlen(str1);
        j += strlen(str2);
      } else {
        newStr[j] = s->value[i];
        i++;
        j++;
      }
    }
    newStr[j] = '\0';
    free(s->value);
    s->value = newStr;
    s->size = j;
    return s;
  });

  s->chomp = s->blockCopy(^(void) {
    if (s->value[s->size - 1] == '\n') {
      s->value[s->size - 1] = '\0';
      s->size--;
    }
    return s;
  });

  s->slice = s->blockCopy(^(size_t start, size_t length) {
    if (start >= s->size) {
      return string("");
    }
    if (start + length > s->size) {
      length = s->size - start;
    }
    char *new_str = malloc(length + 1);
    strncpy(new_str, s->value + start, length);
    new_str[length] = '\0';
    string_t *temp = string(new_str);
    free(new_str);
    return temp;
  });

  s->indexOf = s->blockCopy(^(const char *str) {
    for (int i = 0; i < (int)s->size; i++) {
      if (strncmp(s->value + i, str, strlen(str)) == 0) {
        return i;
      }
    }
    return -1;
  });

  s->lastIndexOf = s->blockCopy(^(const char *str) {
    for (int i = s->size - 1; i > 0; i--) {
      if (strncmp(s->value + i, str, strlen(str)) == 0) {
        return i;
      }
    }
    return -1;
  });

  s->eql = s->blockCopy(^(const char *str) {
    return strcmp(s->value, str) == 0;
  });

  s->contains = s->blockCopy(^(const char *str) {
    return s->indexOf(str) != -1;
  });

  s->split = s->blockCopy(^(const char *delim) {
    string_collection_t *collection = stringCollection(0, NULL);
    char *tknPtr;
    char *token = strtok_r(s->value, delim, &tknPtr);
    while (token != NULL) {
      collection->push(string(token));
      token = strtok_r(NULL, delim, &tknPtr);
    }
    return collection;
  });

  s->matchGroup = s->blockCopy(^(const char *regex) {
    string_collection_t *collection = stringCollection(0, NULL);
    size_t maxMatches = 100;
    size_t maxGroups = 100;
    regex_t regexCompiled;
    regmatch_t groupArray[maxGroups];
    unsigned int m;
    char *cursor;
    if (regcomp(&regexCompiled, regex, REG_EXTENDED)) {
      log_err("regcomp() failed");
    };
    cursor = (char *)s->value;
    for (m = 0; m < maxMatches; m++) {
      if (regexec(&regexCompiled, cursor, maxGroups, groupArray, 0))
        break; // No more matches
      unsigned int g = 0;
      unsigned int offset = 0;
      for (g = 0; g < maxGroups; g++) {
        if (groupArray[g].rm_so == (long long)(size_t)-1)
          break; // No more groups
        char cursorCopy[strlen(cursor) + 1];
        strlcpy(cursorCopy, cursor, groupArray[g].rm_eo + 1);
        if (g == 0) {
          offset = groupArray[g].rm_eo;
        } else {
          char *key = malloc(sizeof(char) *
                             (groupArray[g].rm_eo - groupArray[g].rm_so + 1));
          strlcpy(key, cursorCopy + groupArray[g].rm_so,
                  groupArray[g].rm_eo - groupArray[g].rm_so + 1);
          string_t *temp = string(key);
          free(key);
          collection->push(temp);
        }
      }
      cursor += offset;
    }
    regfree(&regexCompiled);
    return collection;
  });

  s->free = Block_copy(^(void) {
    free(s->value);
    for (int i = 0; i < s->blockCopyCount; i++) {
      Block_release(s->blockCopies[i].ptr);
    }
    Block_release(s->blockCopy);
    dispatch_async(dispatch_get_main_queue(), ^() {
      Block_release(s->free);
      free(s);
    });
  });

  return s;
}
