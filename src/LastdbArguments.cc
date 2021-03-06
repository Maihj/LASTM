// Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014 Martin C. Frith

#include "LastdbArguments.hh"
#include "stringify.hh"
#include <unistd.h>  // getopt
#include <iostream>
#include <stdexcept>
#include <cstdlib>  // EXIT_SUCCESS

#define ERR(x) throw std::runtime_error(x)

static void badopt( char opt, const char* arg ){
  ERR( std::string("bad option value: -") + opt + ' ' + arg );
}

static int myGetopt( int argc, char** argv, const char* optstring ){
  if( optind < argc ){
    std::string nextarg = argv[optind];
    if( nextarg == "--help"    ) return 'h';
    if( nextarg == "--version" ) return 'V';
  }
  return getopt( argc, argv, optstring );
}

using namespace cbrc;

static bool isBisulfite( const std::vector< std::string >& seeds ){
  return seeds.size() == 1 && (seeds[0] == "BISF" || seeds[0] == "BISR");
}

LastdbArguments::LastdbArguments() :
  isProtein(false),
  isKeepLowercase(true),
  tantanSetting(0),
  isCaseSensitive(false),
  seedPatterns(0),
  volumeSize(-1),
  indexStep(0),  // depends on the subset seed
  subsetSeedFiles(0),
  userAlphabet(""),
  minSeedLimit(0),
  bucketDepth(indexT(-1)),  // means: use the default (adapts to the data)
  childTableType(0),
  isCountsOnly(false),
  verbosity(0),
  inputFormat(sequenceFormat::fasta){}

void LastdbArguments::fromArgs( int argc, char** argv ){
  std::string usage = "\
Usage: lastdb [options] output-name fasta-sequence-file(s)\n\
Prepare sequences for subsequent alignment with lastal.\n\
\n\
Main Options:\n\
-h, --help: show all options and their default settings, and exit\n\
-p: interpret the sequences as proteins\n\
-R: repeat-marking options (default="
    + stringify(isKeepLowercase) + stringify(tantanSetting) + ")\n\
-c: soft-mask lowercase letters";

  std::string help = usage + "\n\
\n\
Advanced Options (default settings):\n\
-Q: input format: 0=fasta, 1=fastq-sanger, 2=fastq-solexa, 3=fastq-illumina ("
      + stringify(inputFormat) + ")\n\
-s: volume size (unlimited)\n\
-m: seed pattern\n\
-u: subset seed (yass.seed)\n\
-w: index step\n\
-a: user-defined alphabet\n\
-i: minimum limit on initial matches per query position ("
    + stringify(minSeedLimit) + ")\n\
-b: bucket depth\n\
-C: child table type: 0=none, 1=byte-size, 2=short-size, 3=full ("
    + stringify(childTableType) + ")\n\
-x: just count sequences and letters\n\
-v: be verbose: write messages about what lastdb is doing\n\
-V, --version: show version information, and exit\n\
\n\
Report bugs to: last-align (ATmark) googlegroups (dot) com\n\
LAST home page: http://last.cbrc.jp/\n\
";

  int c;
  while( (c = myGetopt(argc, argv, "hVpR:cm:s:w:u:a:i:b:C:xvQ:")) != -1 ) {
    switch(c){
    case 'h':
      std::cout << help;
      throw EXIT_SUCCESS;
    case 'V':
      std::cout << "lastdb "
#include "version.hh"
	"\n";
      throw EXIT_SUCCESS;
    case 'p':
      isProtein = true;
      break;
    case 'R':
      if( optarg[0] < '0' || optarg[0] > '1' ) badopt( c, optarg );
      if( optarg[1] < '0' || optarg[1] > '2' ) badopt( c, optarg );
      if( optarg[2] ) badopt( c, optarg );
      isKeepLowercase = optarg[0] - '0';
      tantanSetting = optarg[1] - '0';
      break;
    case 'c':
      isCaseSensitive = true;
      break;
    case 'm':
      seedPatterns.push_back(optarg);
      break;
    case 's':
      unstringifySize( volumeSize, optarg );
      break;
    case 'w':
      unstringify( indexStep, optarg );
      if( indexStep < 1 ) badopt( c, optarg );
      break;
    case 'u':
      subsetSeedFiles.push_back(optarg);
      break;
    case 'a':
      userAlphabet = optarg;
      break;
    case 'i':
      unstringify( minSeedLimit, optarg );
      break;
    case 'b':
      unstringify( bucketDepth, optarg );
      break;
    case 'C':
      unstringify( childTableType, optarg );
      if( childTableType < 0 || childTableType > 3 ) badopt( c, optarg );
      break;
    case 'x':
      isCountsOnly = true;
      break;
    case 'v':
      ++verbosity;
      break;
    case 'Q':
      unstringify( inputFormat, optarg );
      if( inputFormat >= sequenceFormat::prb ) badopt( c, optarg );
      break;
    case '?':
      ERR( "bad option" );
    }
  }

  if( indexStep == 0 ) indexStep = isBisulfite( subsetSeedFiles ) ? 2 : 1;
  // This is because the bisulfite recipe uses two indexes at once, so
  // it's more important to save memory.  In a test, this did not harm
  // sensitivity, and even seemed to improve it.

  if( optind >= argc )
    ERR( "please give me an output name and sequence file(s)\n\n" + usage );
  lastdbName = argv[optind++];
  inputStart = optind;
}
