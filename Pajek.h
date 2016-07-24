/* ========================================================================== */
/*                                                                            */
/*   Pajek.h                                                                  */
/*   (c) 2016 Frederik Schaff <frederik.schaff@fernuni-hagen.de               */
/*                                                                            */
/*   Write Pajek file of Network. 							                              */
/*   Format used here is that for pajekToSvgAnim package                      */
/*                                                                            */
/*   Originally created to use with LSD. Only tested with LSD on windows.     */
/*															(https://github.com/marcov64/Lsd)             */
/*                                                                            */
/*   1) Errors and Infos are printed to the output defined as                 */
/*			PAJEK_MSG(pajek_msg)  if provided. Else nothing is printed.           */
/*  2a)Initialise a new network via pajek_init()                              */
/*   b)Add different kinds of arcs/edges if wanted via                        */
/*    Pajek_init_KindsOfRelations() (after pajek_init() ).                    */
/*   c)Each time you add arcs, edges or vertices use the according commands   */
/*	 d)At the end of a time-period, use the command pajek_snapshot().         */
/*	 e)At the end of the simulation, use the pajek_close() command.           */
/*                                                                            */
/* 3) if no "time" is provided, it is started with time = 1 and time is       */
/*		automatically increased at each snappshot. One may create "initiale"    */
/*		data with an additional call using pajek_snapshot() with time=0.        */
/*		If no "serial" is provided, the first pajek_ini sets it to one and it is*/
/*		increased afterwards by one, each time a new initialisation is done.    */
/*                                                                            */
/*                                                                            */
/*   See the description of the #defines here and the pajek_ functions in the */
/* 	 accompanying .cpp file for more information.                             */
/* ========================================================================== */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

/* Error and other msg's output */
#ifndef PAJEK_MSG
	#define PAJEK_MSG(MSG) void() /* Provide some way to channel the msgs! */
#endif

#ifndef PAJEK_MAX_VERTICES
	#define PAJEK_MAX_VERTICES 1000  /* The maximal number of vertices */
#endif

#ifndef PAJEK_MAX_SNAPSHOTS
	#define PAJEK_MAX_SNAPSHOTS 200
#endif

/* The maximal number of diff. relations for each, arcs and edge.
	Only relevant	if the grouped format is chosen. */
#ifndef PAJEK_KINDS_RELATIONS
	#define PAJEK_KINDS_RELATIONS 2
#endif

/* The maximal number of combined arcs and edges per time. Default should
	suffice in most cases.*/
#ifndef PAJEK_MAX_ARCS
	#define PAJEK_MAX_ARCS \
				PAJEK_MAX_VERTICES*PAJEK_MAX_VERTICES*2*PAJEK_KINDS_RELATIONS
#endif

/* An additional dirname for the copies. Note that this works not in win64
	currently. */
#ifndef PAJEK_DIRNAME
	#define PAJEK_DIRNAME "Network"
#endif
/* A default filename */
#ifndef PAJEK_FILENAME
	#define PAJEK_FILENAME "netw"
#endif

/* I guess (?) it is useful to sometimes provide an interval even if its a
	snapshot if one wants to use the PajekToSVGAnim pakage. */
#ifndef PAJEK_FORCE_COMPLETE
	#define PAJEK_FORCE_COMPLETE true
#endif

/* Pajek files will be split if they become very large, but only if no single
	complete file is warranted. */
#ifndef PAJEK_SNAPSHOTS_PER_FILE
		#define PAJEK_SNAPSHOTS_PER_FILE 100
#endif

/* Create Directory -- currently only working in linux (not tested) and
	win32 (tested) but NOT win64(not tested) with cygwin64 */
#ifndef NEW_DIR
	#if defined(_WIN32)
		#define NEW_DIR(str) _mkdir(str)
	#else
		#define NEW_DIR(str) void()
		//mkdir(str, 0777);
	#endif
#endif

/* from equal named LSD command */
#define FOLDERSEP( dir ) ( dir[0] == '\0' ? "" : "/" )

/* Some global variables */
char pajek_msg[196];
char const pajek_dirname[] = PAJEK_DIRNAME;
char const pajek_filename[] = PAJEK_FILENAME;
char pajek_dir_path[64];
char pajek_path[300];
char pajek_filename_suffix[64];
char pajek_dirname_suffix[196];
char pajek_buffer[192];
bool pajek_append_mode;


/* To speed things up, the file will be kept open until to the end. */
std::ofstream pajek_file;
std::ifstream pajek_file_in;

int pajek_time;
int pajek_serial=0;

int pajek_snapshot_count;
int pajek_snapshot_parts;

int pajek_vertices_count;
int pajek_vertices_ID[PAJEK_MAX_VERTICES];
char pajek_vertices_label[PAJEK_MAX_VERTICES][16];
int pajek_vertices_shape[PAJEK_MAX_VERTICES];
char pajek_vertices_colour[PAJEK_MAX_VERTICES][16]; //see pajek manual!
double pajek_vertices_xfact[PAJEK_MAX_VERTICES]; //relative size
double pajek_vertices_yfact[PAJEK_MAX_VERTICES]; //relative size
double pajek_vertices_value[PAJEK_MAX_VERTICES]; //what for?
int pajek_vertices_interval_start[PAJEK_MAX_VERTICES]; //Beginning of current activity inteval
int pajek_vertices_interval_end[PAJEK_MAX_VERTICES]; //Beginning of current activity inteval

//arcs are "special" edges.
int pajek_arcs_count;
int pajek_edges_count;
int pajek_arcs_edges_count;
bool pajek_arcs_isEdge[PAJEK_MAX_ARCS];  //0/false: Arc, else Edge
int pajek_arcs_source[PAJEK_MAX_ARCS];
int pajek_arcs_target[PAJEK_MAX_ARCS];
double pajek_arcs_value[PAJEK_MAX_ARCS];
char pajek_arcs_label[PAJEK_MAX_ARCS][16];
int pajek_arcs_width[PAJEK_MAX_ARCS];
char pajek_arcs_colour[PAJEK_MAX_ARCS][16];
int pajek_arcs_interval_start[PAJEK_MAX_ARCS]; //Beginning of current activity inteval
int pajek_arcs_interval_end[PAJEK_MAX_ARCS];

char pajek_arcs_kindsOf[PAJEK_KINDS_RELATIONS][16]; //Alternatively, provide number of different relations
int  pajek_arcs_kindsOf_count=0;
char pajek_edges_kindsOf[PAJEK_KINDS_RELATIONS][16]; //Alternatively, provide number of different relations
int  pajek_edges_kindsOf_count=0;

/* A set of arrays to hold the time-stamps for each arc/edge
*/
int pajek_timeline[PAJEK_MAX_SNAPSHOTS]; //for each snapshot, hold the time-info
// For each arc/edge there is a unique entry in *_source[t]~*_target[t]~*_kind[t]
// and in the accompanying *_timeStamps[t] it will be true/false if
//active/not-active
bool pajek_arcs_timeStamps[PAJEK_MAX_ARCS][PAJEK_MAX_SNAPSHOTS];
int pajek_arcs_timeStamps_source[PAJEK_MAX_ARCS];
int pajek_arcs_timeStamps_target[PAJEK_MAX_ARCS];
char pajek_arcs_timeStamps_kind[PAJEK_MAX_ARCS][16];
bool pajek_arcs_timeStamps_isEdge[PAJEK_MAX_ARCS];
//Similar for vertices.
bool pajek_vertices_timeStamps[PAJEK_MAX_VERTICES][PAJEK_MAX_SNAPSHOTS];
int pajek_vertices_timeStamps_id[PAJEK_MAX_VERTICES];
char pajek_vertices_timeStamps_label[PAJEK_MAX_VERTICES][16];

/* The functions */

bool pajek_init( int serial, bool append=true, char const *dirname_suffix="", char const *filename_suffix="" );

bool pajek_vertices_add( int ID, char const label[]="Vertice",  int shape=0, char const *colour="Black", double x_fact=1.0, double y_fact=-1.0, double value=1.0, int start=-1, int end=-1 );
bool pajek_arcs_add( bool isedge, int source, int target, double value, char const *label="EdgeOrArc",  int width=1, char const *colour="Black", int start=-1, int end=-1 );


bool pajek_write( int time=-1);
bool pajek_append (int time=-1,  bool final=false);
bool pajek_snapshot(int time, bool final=false);

void pajek_clear();
bool pajek_openFile( bool append, int serial, int time=-1);
bool pajek_close( bool final=false, bool append=false);
bool pajek_partial_snaps( bool renew);

bool pajek_init_KindsOfRelation( char const *relation, bool isedge);

char *pajek_shape( int shape=0);
bool pajek_relative_xy( double tau, double *pos_x,double *pos_y);

bool pajek_init_timeline();
bool pajek_vertices_timeline_add(int ID, char const *label, int snapshot);
bool pajek_arcs_timeline_add(int Source, int Target, char const *Kind, int snapshot, bool isEdge);
bool pajek_timeline_close();

