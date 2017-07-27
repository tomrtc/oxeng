
#include <cstdlib>
#include <iostream>

#include <algorithm>
#include <chrono>
#include <functional>
#include <vector>
#include <regex>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include <map>
//#include <unordered_map>
//#include <utility>
#include <initializer_list>
#include <thread>
// C/Posix/linux headers. 
// no more necessary #define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <libgen.h> //basename(3) posix
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <stdlib.h>
#include <sys/stat.h>


//header only 3rd party libraries.

#include "prettyprint.hh"

#include <oxeng.hh>

// forward prototypes.
void show_smiley(void);
void show_usage(const char *progname);




int
main (int argc,char *argv[])
{
  char     optchar;
  builder_context builder;

  // avoid useless compatibilty with C.								
  std::ios_base::sync_with_stdio(false);			
  std::cin.tie(NULL);
 
  std::chrono::high_resolution_clock::time_point
    tstart = std::chrono::high_resolution_clock::now();

  if(argc == 1)
    {
      show_usage(argv[0]);
      exit(1);
    }

  try {
    while((optchar=getopt(argc,argv,"hvds:u:"))!=-1)
      {
	switch(optchar)
	  {      
	  case 'h':/*option h show the help*/
	    show_usage(argv[0]);
	    break;
         
	           
	  case 'u': /*-u $utl_path */
	    builder.set_utl_directory(optarg);
	    break;

	  case 's':/*-t $strings configuration and data translation files directory*/
	    builder.set_string_directory(optarg);
	    break;

	  case 'v':/*display the version.*/
	    std::cout<<"The current version is 0.1."<<std::endl;
	    break;
          
	  case 'd': /* debug and fun!*/
	    builder.set_debug();  
	    break;

	  default:
	    show_usage(argv[0]);
	    break;
	  }
      }
  } catch (int e)
  {
    std::cerr  << " : " << strerror(errno) << std::endl;
    return e;
  }
  catch (std::exception& e)
  {
    std::cerr << "exception caught: " << std::endl;
  }
  try {
    builder.parse_tel_targets_async();
    builder.generate_tel_binary_tsl_async();
    builder.parse_all_applications();
  } catch (std::string e)
  {
    std::cerr  << e << " : " << strerror(errno) << std::endl;
    return errno;
  }

          
  int hardwareThreads = std::thread::hardware_concurrency();// number of threads supported by the hardware
  std::chrono::high_resolution_clock::time_point
    tstop = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double>
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tstop - tstart);
  std::cout << "\nCPUs " << sysconf(_SC_NPROCESSORS_ONLN) << " - " << hardwareThreads <<" Time elapsed: " << time_span.count() << " s" << std::endl;
  return 0;
}


void
show_usage(const char *s)
{
  std::cout<<"Usage:   "<< s <<" [-option] [argument]"<<std::endl;
  std::cout<<"option:  "<<"-h show help information"<<std::endl;

  std::cout<<"         "<<"-s string path data translation files directory (*.FR0 ...) and config directory (should contains lang.cnf ...)"<<std::endl;
  std::cout<<"         "<<"-u utl should contains loc_str.doc ..."<<std::endl;
  std::cout<<"         "<<"-v show version infomation"<<std::endl;
  std::cout<<"example: "<< s <<" -u $utl_path -s $strings_path -d"<<std::endl;
}





const char *smiley =
								    R"SMYLE(                   ,ijfLLLLfji,.
              ;fWKK##############DL;
           ;LDEKKDKKK###########EK##KG;
         tWEEEGi.  ;DKK#######j.  :G###Kj
       ,GDDDED:      WKK#####;      L####Ki
      tDDDDDDi       :KKKK##L        K#####L
     fWWWDDDD.        DKKKK#;        G######D.
    fWWWWWWDW         LKKKKK:        j#######D
   :GGWWWWWWW         GEEKKK,        f########;
   jGGGGWWWWW,        DEEEKKt        D########D
   LGGGGGWWWWf       tEEEEEED.      ;##########:
  .LLGGGGGGWWWt     tDDDEEEEEG,    iE##########;
   LLLLGGGGGGWWGjijGDDDDDEEEEEEGfLDKKK#########.
   tLLLLLGGGGGGWWWWWWDDDDDDEEEEEEKKKKKK#######D
   .LLLLLLGGGGGGGWWWWWWDDDDDEEEEEEEKKKKKK#####;
    tfLLLLi :;LGGGWWWWWWWDDDDDEEEEEEWttEKKK##W
     ifLLLfi.  ,tfGGWWWWWWDDDDDDEDL;  ,DKKKKW.
      ;ffLLLLf;   .,tfGWWWWDDWLt,   ;LEEKKKf
       :jffLLLLLLt;.            :tLDEEEEEW,
         ,jfLLLLLLLGLfjjjjjjjfGWDDDDDEEWi
           :ijLLLLLLGGGGGGWWWWWWDDDDWf,
              :;tLLLLLGGGGGGWWWWWLt:
                   .,;;iitii;,:)SMYLE";

void show_smiley(void)
{
  std::cout<<smiley<<std::endl;
}


