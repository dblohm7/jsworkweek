/*
 * This define is for Windows only, it is a work-around for bug 661663.
 */
#ifdef _MSC_VER
# define XP_WIN
# include <windows.h>
#endif

#include "jsapi.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NS_PER_SEC 1000000000LL

/* The class of the global object. */
static JSClass global_class = {"global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub,
                               JS_DeletePropertyStub, JS_PropertyStub, 
                               JS_StrictPropertyStub, JS_EnumerateStub, 
                               JS_ResolveStub, JS_ConvertStub, NULL, 
                               JSCLASS_NO_OPTIONAL_MEMBERS};

/* The error reporter callback. */
void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
  fprintf(stderr, "%s:%u:%s\n",
          report->filename ? report->filename : "<no filename="">",
          (unsigned int) report->lineno,
          message);
}

int main(int argc, const char *argv[])
{
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <script> <iterations>\n", argv[0]);
    return 1;
  }

  /* JS variables. */
  JSRuntime *rt;
  JSContext *cx;
  JSObject *global;

  /* Create a JS runtime. */
  rt = JS_NewRuntime(8L * 1024L * 1024L, JS_NO_HELPER_THREADS);
  if (rt == NULL)
    return 1;

  /* Create a context. */
  cx = JS_NewContext(rt, 8192);
  if (cx == NULL)
    return 1;
  JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT);
  JS_SetVersion(cx, JSVERSION_LATEST);
  JS_SetErrorReporter(cx, reportError);

  /* Create the global object in a new compartment. */
  global = JS_NewGlobalObject(cx, &global_class, NULL);
  if (global == NULL)
    return 1;

  /* Populate the global object with the standard globals, like Object and Array. */
  if (!JS_InitStandardClasses(cx, global))
    return 1;

  /* Your application code here. This may include JSAPI calls to create your own custom JS objects and run scripts. */
  unsigned int iterations = strtoul(argv[2], nullptr, 10);
  if (!iterations) {
    return 1;
  }

  struct stat scriptStat;
  if (stat(argv[1], &scriptStat) < 0) {
    return 1;
  }

  FILE* scriptFile = fopen(argv[1], "r");
  if (!scriptFile) {
    return 1;
  }

  char *buf = (char*) malloc(scriptStat.st_size);
  size_t readResult = fread(buf, 1, scriptStat.st_size, scriptFile);
  if (readResult < scriptStat.st_size && ferror(scriptFile)) {
    free(buf);
    fclose(scriptFile);
    return 1;
  }
  fclose(scriptFile);

  /* buf holds the script. Now we can bytecode compile and such */
  JSScript* script = nullptr;
  LARGE_INTEGER start, end, freq;
  if (!QueryPerformanceFrequency(&freq)) {
    fprintf(stderr, "Warning: QueryPerformanceFrequency failed\n");
  }
  if (!QueryPerformanceCounter(&start)) {
    fprintf(stderr, "Warning: Counter failed\n");
  }
  for (unsigned int i = 0; i < iterations; ++i) {
    script = JS_CompileScript(cx, global, buf, readResult, argv[1], 1);
    if (!script) {
      fprintf(stderr, "Script bytecode compilation failed\n");
      break;
    }
  }
  if (!QueryPerformanceCounter(&end)) {
    fprintf(stderr, "Warning: Counter failed\n");
  }
  LARGE_INTEGER result1;
  result1.QuadPart = ((end.QuadPart - start.QuadPart) * NS_PER_SEC / freq.QuadPart) / iterations;
  printf("%lld ns per iteration\n", result1.QuadPart);
  uint32_t encodedLen = 0;
  void* encodedScript = JS_EncodeScript(cx, script, &encodedLen);
  if (encodedScript) {
    if (!QueryPerformanceCounter(&start)) {
      fprintf(stderr, "Warning: Counter failed\n");
    }
    for (unsigned int i = 0; i < iterations; ++i) {
      script = JS_DecodeScript(cx, encodedScript, encodedLen, nullptr, nullptr);
      if (!script) {
        fprintf(stderr, "Script decoding failed\n");
        break;
      }
    }
    if (!QueryPerformanceCounter(&end)) {
      fprintf(stderr, "Warning: Counter failed\n");
    }
    LARGE_INTEGER result2;
    result2.QuadPart = ((end.QuadPart - start.QuadPart) * NS_PER_SEC / freq.QuadPart) / iterations;
    printf("%lld ns per iteration\n", result2.QuadPart);
    double dresult1 = (double)result1.QuadPart;
    double dresult2 = (double)result2.QuadPart;
    printf("Decoding is %.3fx faster\n", dresult1 / dresult2);
  } else {
    fprintf(stderr, "Script encoding failed\n");
  }

  // js_free(encodedScript);
  free(buf);

  /* JS Cleanup */
  JS_DestroyContext(cx);
  JS_DestroyRuntime(rt);
  JS_ShutDown();
  return 0;
}

