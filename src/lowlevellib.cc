#include "lowlevellib.hh"

#include <cstdlib>

#include <vector>
#include <string>
#include <unordered_map>
#include <regex>
//header only 3rd party libraries.
#include <iostream>     // std::cout, std::ostream, std::ios

#include <fstream>      // std::filebuf
#include <sstream>
#include <iomanip>      // std::setw

// C/Posix/linux headers.
// no more necessary #define _XOPEN_SOURCE 500
#include <stdio.h>
#include <libgen.h> //basename(3) posix
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <stdlib.h>
#include <sys/stat.h>



std::string get_file_contents(const std::string filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
    {
      std::ostringstream contents;
      contents << in.rdbuf();
      in.close();
      return(contents.str());
    }
  throw(filename);
}

bool
file_exists(const char *path)
{
  struct stat fstat;
  if ( stat(path, &fstat) )
    {
      std::cerr << path << " :" << strerror(errno) << std::endl;
      return false;
    }
  if ( !S_ISREG(fstat.st_mode) )
    {
      errno = EISDIR;
      return false;
    }
  return true;
}


bool
directory_exists(const char *path)
{
  struct stat fstat;
  if ( stat(path, &fstat) )
    {
      std::cerr << path << " :" << strerror(errno) << std::endl;
      return false;
    }
  if ( !S_ISDIR(fstat.st_mode) )
    {
      errno = ENOTDIR;
      return false;
    }

  return true;
}
