#include <nmealib/context.h>
#include <nmealib/gmath.h>
#include <nmealib/info.h>
#include <nmealib/parser.h>
#include <nmealib/sentence.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>

static size_t countlines(char * line) {
  size_t cnt = 0;
  char * s = line;

  while (*s && (s = strchr(s, '\n'))) {
    s++;
    cnt++;
  }

  return cnt;
}

static size_t printInfo(char * inputLine, char * outputLine, size_t outputLineResult, NmeaInfo * info, char * outputbuffer, size_t outputbuffersize) {

#define dst       (&outputbuffer[index])
#define available ((size_t) MAX((long) outputbuffersize - index - 1, 0))

  size_t i;
  int index = 0;
  size_t lineCount = 0;
  int eq;

  index += snprintf(dst, available, "\n%-5s%-8lu: %s\n", "", (unsigned long) strlen(inputLine), inputLine);
  lineCount += 3;

  eq = !strcmp(inputLine, outputLine);

  index += snprintf(dst, available, "%-5s%-8lu: %s\n", eq ? "EQ" : "NEQ", (unsigned long) outputLineResult, outputLine);
  lineCount += 1 + countlines(outputLine);

  index += snprintf(dst, available, "  %s = %d/%d\n", "sig/fix", info->sig,
      info->fix);
  index += snprintf(dst, available, "  %s = %04u%02u%02u %02u:%02u:%02u.%02u\n",
      "utc", info->utc.year, info->utc.mon, info->utc.day, info->utc.hour, info->utc.min, info->utc.sec,
      info->utc.hsec);
  index += snprintf(dst, available, "  %s = %08x\n", "smask", info->smask);
  index += snprintf(dst, available, "  %s = %08x\n", "present", info->present);
  index += snprintf(dst, available, "  %s = %f/%f\n", "lat/lon", info->lat,
      info->lon);
  index += snprintf(dst, available, "  %s = %f/%f\n", "speed/elv", info->speed,
      info->elv);
  index += snprintf(dst, available, "  %s = %f/%f/%f\n", "track/mtrack/magvar",
      info->track, info->mtrack, info->magvar);
  index += snprintf(dst, available, "  %s = %f/%f/%f\n", "hdop/pdop/vdop",
      info->hdop, info->vdop, info->pdop);
  lineCount += 8;

  index += snprintf(dst, available, "  %s\n", "satinfo");
  index += snprintf(dst, available, "    %s = %d/%d\n", "inuse/inview",
      info->satinfo.inUseCount, info->satinfo.inViewCount);
  lineCount += 2;

  for (i = 0; i < NMEALIB_MAX_SATELLITES; i++) {
    index += snprintf(dst, available, "    %02lu %s = %d/%d/%d/%d/%d\n", (long unsigned) i,
        "in_use/id/sig/elv/azimuth", info->satinfo.inUse[i], info->satinfo.inView[i].prn, info->satinfo.inView[i].snr,
        info->satinfo.inView[i].elevation, info->satinfo.inView[i].azimuth);
    lineCount += 1;
  }

  return lineCount;
}

static void nmeaErrorLocal(const char *s, size_t sz __attribute__((unused))) {
  fprintf(stderr, "ERROR: %s\n", s);
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
  int exitCode = EXIT_SUCCESS;
  char expectedFileName[2048];
  char inputFileName[2048];
  FILE * expectedFile;
  FILE * inputFile;
  char * expectedLine = NULL;
  char * inputLine = NULL;
  char * outputLine = NULL;
  size_t expectedLineLength = 0;
  size_t inputLineLength = 0;
  size_t outputLineLength = 0;
  ssize_t expectedLineCount;
  ssize_t inputLineCount;
  const char * defaultFileName;
  const char * directoryName;
  NmeaInfo info;
  nmeaPARSER parser;
  char expectedbuffer[65536];
  size_t expectedbufferlength = 0;
  char outputbuffer[65536];
  size_t expectedLineIndex = 0;
  bool dooutput = false;
  long inputLineNr = 0;
  long expectedLineNr = 1;

  inputLineLength = 4096;
  outputLineLength = 4096;
  inputLine = malloc(inputLineLength);
  outputLine = malloc(outputLineLength);
  if (!inputLine || !outputLine) {
    exitCode = EXIT_FAILURE;
    goto out;
  }

  directoryName = dirname(dirname(argv[0]));
  defaultFileName = "/parse_test/nmea.txt";
  if (argc > 1) {
    if (!strcmp("stdout", argv[1])) {
      dooutput = true;
    } else {
      directoryName = "";
      defaultFileName = argv[1];
    }
  }
  snprintf(expectedFileName, sizeof(expectedFileName), "%s%s.expected", directoryName, defaultFileName);
  snprintf(inputFileName, sizeof(inputFileName), "%s%s", directoryName, defaultFileName);
  fprintf(stderr, "Using files %s and %s\n", inputFileName, expectedFileName);

  inputFile = fopen(inputFileName, "r");
  if (inputFile == NULL) {
    fprintf(stderr, "Could not open file %s\n", inputFileName);
    exitCode = EXIT_FAILURE;
    goto out;
  }
  expectedFile = fopen(expectedFileName, "r");
  if (expectedFile == NULL) {
    fprintf(stderr, "Could not open file %s\n", expectedFileName);
    exitCode = EXIT_FAILURE;
    goto out;
  }

  nmeaContextSetErrorFunction(nmeaErrorLocal);

  nmea_parser_init(&parser);

  inputLineNr = 0;
  expectedLineNr = 1;

  while ((inputLineCount = getline(&inputLine, &inputLineLength, inputFile)) != -1) {
    size_t outputLineResult;
    size_t lineCount;

    inputLineNr++;

    memset(&info, 0, sizeof(info));

    nmea_parse(&parser, inputLine, (size_t) inputLineCount, &info);

    outputLineResult = nmeaSentenceFromInfo(outputLine, outputLineLength, &info, info.smask);

    lineCount = printInfo(inputLine, outputLine, outputLineResult, &info, outputbuffer, sizeof(outputbuffer));

    if (dooutput) {
      printf("%s", outputbuffer);
    } else {
      long linesInExpected = 0;
      // read expected
      expectedbuffer[0] = '\0';
      expectedbufferlength = 0;
      for (expectedLineIndex = 0; expectedLineIndex < lineCount; expectedLineIndex++) {
        if ((expectedLineCount = getline(&expectedLine, &expectedLineLength, expectedFile)) != -1) {
          linesInExpected++;
          strncpy(&expectedbuffer[expectedbufferlength], expectedLine, (size_t) expectedLineCount);
          expectedbufferlength += (size_t) expectedLineCount;
        }
      }
      expectedbuffer[expectedbufferlength] = '\0';

      if (strcmp(outputbuffer, expectedbuffer)) {
        printf("FAILED\n");
        printf("\n  ACTUAL on line %ld:\n", inputLineNr);
        printf("%s", outputbuffer);
        printf("\n  EXPECTED on line %ld:\n", expectedLineNr);
        printf("%s", expectedbuffer);
        exitCode = EXIT_FAILURE;
        goto out;
      }

      expectedLineNr += linesInExpected;
    }
  }

  if (!dooutput) {
    printf("SUCCESS\n");
  }

  if (inputFile)
    fclose(inputFile);
  if (expectedFile)
    fclose(expectedFile);

  exitCode = EXIT_SUCCESS;

out:
  free(inputLine);
  free(outputLine);
  exit(exitCode);
}
