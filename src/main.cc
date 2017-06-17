
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
    while((optchar=getopt(argc,argv,"hvdi:c:t:"))!=-1)
      {
	switch(optchar)
	  {      
	  case 'h':/*option h show the help*/
	    show_usage(argv[0]);
	    break;
         
	  case 'i':  /*-i str_filename(loc_str.doc)*/
	    builder.set_string_path(optarg);
	    break;
         
	  case 'c': /*-c config directory , load config files directly*/
	    builder.set_configuration_directory(optarg);
	    break;

	  case 't':/*-t data translation files directory*/
	    builder.set_data_directory(optarg);
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
    //builder.parse_target();
    std::string   gea{"GEA"};
    std::string   fr0{"FR0"};
    std::string   gr0{"GR0"};
    //builder.load_tsl_async(gea);
    // builder.load_all_tsl(fr0);
    // builder.load_all_tsl(gr0);
   // builder.load_def();
 
    builder.generate_locate();
  //    builder.generate_stu(gea);
    // builder.generate_stu(fr0);
    //builder.generate_stu(gr0);
  //builder.generate_ref();
   builder.generate_tel_binary_tsl_async();
  } catch (std::string e)
  {
    std::cerr  << e << " : " << strerror(errno) << std::endl;
    return errno;
  }
#if 0  
  std::cout << m1 << std::endl;
  std::cout << l1 << std::endl;
  std::cout << f1 << std::endl;
#endif
          
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
  std::cout<<"         "<<"-i docfilename (eg: loc_str.doc)"<<std::endl;
  std::cout<<"         "<<"-c config directory (should contains lang.cnf ...)"<<std::endl;
  std::cout<<"         "<<"-t data translation files directory (*.FR0 ...)"<<std::endl;
  std::cout<<"         "<<"-v show version infomation"<<std::endl;
  std::cout<<"example: "<< s <<" -i loc_str.doc -c config -t data"<<std::endl;
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


