#ifndef STRNDUP_H__
#define STRNDUP_H__

char* strndup(const char *s, size_t size)
{
  char *r;
  char *end = (char*)memchr(s, 0, size);
  
  if (end)
    /* Length + 1 */
    size = end - s + 1;
  
  r = (char*)malloc(size);

  if (size)
    {
      memcpy(r, s, size-1);
      r[size-1] = '\0';
    }
  return r;
}


#endif // STRNDUP_H__
