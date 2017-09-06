# PajekFromCpp
Stand alone c++ source code to create pajek .net and .paj (including timeline feature for PajekToScgAnim) files from any simulation in c/c++.

This code is generated and tested for the use with LSD by Marco Valente (http://www.labsimdev.org/Joomla_1-3/, https://github.com/marcov64/Lsd), a c++ based framework for agent-based modeling. It is not a substitute to the network functionality implemented by Marcelo Pereira in current snapshots of LSD. It is only ment to be used to export network-data from a simulation in such a manner that it may be analysed and visualised with pajek or other software that can make use of the .paj or .net format.  The intention is to provide full flexibility and all the options that pajek by Andrej Mrvar (http://mrvar.fdv.uni-lj.si/pajek/) .paj (and .net) format provides. In addition, the .paj files created may be used with PajekToScgAnim by Darko Brvar: - PajekToSvgAnim (by Darko Brvar): http://mrvar.fdv.uni-lj.si/pajek/PajekToSvgAnim/PajekToSvgAnim.zip - test datasets: http://mrvar.fdv.uni-lj.si/pajek/PajekToSvgAnim/AnimData.zip - manual: http://freeweb.siol.net/dbrvar2/PajekToSvgAnim11081.pdf 

### Usage

#### Inclusion in LSD
Include the pajek.cpp, which will in turn include the header file. In LSD you need to put (change as apropriate)
	#include "PajekFromCpp/Pajek.cpp"
just before ==MODELBEGIN== in the fun_\*\*.cpp file.

#### How to use it
##### General Info
- Errors and Infos are printed to the output defined as ==PAJEK_MSG(pajek_msg)==  if provided. Else nothing is printed.

- There are two modes available: single snapshot mode and append mode:
-- single snapshot mode: Each snapshot is saved in a single .net
-- append mode:
-- a) A (set of) *.paj files is created, holding all the snapshots. Via ==#define PAJEK_MAX_SNAPSHOTS== the number of snapshots per file *_part*.pay is defined. (default 200) 
-- b) ALTERNATIVELY, if ==PAJEK_FORCE_COMPLETE== is defined true (default), a single *.paj file is created holding all snapshots and at the beginning of this file a special snapshot is created, holding all the time-line information, thus making it suitable to work with e.g. PajekToSvgAnim

All the ==#define== are set in the Pajek.h file.

#####Usage (within LSD)

1. At the beginning of a new simulation run, initialise a new network via pajek_init()
2. Add different kinds of arcs/edges if wanted via Pajek_init_KindsOfRelations(), after pajek_init().
3. Each time you add arcs, edges (arc with ==isedge=true==) or vertices use the according commands:

 ```pajek_arcs_add( bool isedge, int source, int target, double value, char const *label="EdgeOrArc",  int width=1, char const *colour="Black", int start=-1, int end=-1 )```
  
 ```pajek_vertices_add( int ID, char const label[]="Vertice",  int shape=0, char const *colour="Black", double x_fact=1.0, double y_fact=-1.0, double value=1.0, double x_pos=-1.0, double y_pos=-1.0, int start=-1, int end=-1 )```
    
4. At the end of a time-period, use the command ==pajek_snapshot(t)==. Or - in case you do not want to save this period - use ==pajek_clear()== (It is better to only gather data when you want to save it)
5. At the end of the simulation, use the ==pajek_snapshot(t,final)== command.

Note that if no "time" is provided (e.g. ==pajek_snapshot()==), it is automatically started with time = 1 and time is automatically increased at each snappshot. One may create "initiale" data with an additional call using ==pajek_snapshot(0)==. If no "serial" is provided, the first pajek_ini sets it to one and it is increased afterwards by one, each time a new initialisation is done (e.g. in a row of simulations in LSD)

See the description of the #defines here and the pajek_ functions in the accompanying .cpp file for more information. Note: Not all functionality is documented here (yet)

Notes regarding PajekToSVGAnim:
Always make sure that at least one arc/edge exists at each time-slice!






### To do's
- Convert to more convenient class structure and using vectors instead of arrays
- Provide thorough comments in the code
- Provide a real manual

###Misc
The code is provided "as is". You may reach out to the author via E-Mail at: [frederik.schaff@fernuni-hagen.de](frederik.schaff@fernuni-hagen.de)

For further information on Pajek see also:

Mrvar, Andrej; Batagelj, Vladimir (2016): Analysis and visualization of large
networks with program package Pajek. In: Complex Adaptive Systems Modeling 4(1),
 S. 1-8. DOI: 10.1186/s40294-016-0017-8.

Nooy, Wouter de; Mrvar, Andrej; Batagelj, Vladimir (2011): Exploratory social
network analysis with Pajek. Rev. and expanded 2nd ed. England, New York:
Cambridge University Press (Structural analysis in the social sciences, 34).
