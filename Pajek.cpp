/* ========================================================================== */
/*                                                                            */
/*   Pajek.cpp                                                                */
/*   (c) 2016 Frederik Schaff <frederik.schaff@fernuni-hagen.de               */
/*                                                                            */
/*   Write Pajek file of Network. 							                              */
/*   Format used here is that for pajekToSvgAnim package                      */
/*   See the acompanying header file for information and definitions          */
/* ========================================================================== */

#include "pajek.h" //Header


/* Reset the counters, starting a new snapshot. */
void pajek_clear(){
  pajek_vertices_count=0;
  pajek_arcs_count=0;
	pajek_edges_count=0;
  pajek_arcs_edges_count=0;
}

/* Add a vertice */
bool pajek_vertices_add(int ID, char const label[],  int shape, char const *colour, double x_fact, double y_fact, double value, int start, int end )
{

	if (pajek_vertices_count == PAJEK_MAX_VERTICES) {
    PAJEK_MSG("\nERROR! Maximum number of vertices reached");
		return false;
	}

  pajek_vertices_ID[pajek_vertices_count]	= ID;
  snprintf(pajek_vertices_label[pajek_vertices_count],sizeof(char)*16,"%s",label); //Check
  pajek_vertices_shape[pajek_vertices_count] = shape;
	//0=ellipse, 1=box, 2=diamond, 3=triangle, 4=cross, 5=empty, 6=house, 7=man, or 8=woman
  snprintf(pajek_vertices_colour[pajek_vertices_count],sizeof(char)*16,"%s",colour);
  pajek_vertices_xfact[pajek_vertices_count] = x_fact;
  pajek_vertices_yfact[pajek_vertices_count] = y_fact>0?y_fact:x_fact;
  pajek_vertices_value[pajek_vertices_count] = value;
	pajek_vertices_interval_start[pajek_vertices_count] = start;
  pajek_vertices_interval_end[pajek_vertices_count] = end;

  pajek_vertices_count++;
 	return true;
}

/* Add an arc or edge */
bool pajek_arcs_add(bool isedge, int source, int target, double value, char const *label,  int width, char const *colour, int start, int end )
{
	if (pajek_arcs_edges_count == PAJEK_MAX_ARCS) {
		PAJEK_MSG("\nERROR! Maximum number of Arcs reached");
		return false;
	}

  pajek_arcs_isEdge[pajek_arcs_edges_count] = isedge;
  pajek_arcs_source[pajek_arcs_edges_count] = source;
  pajek_arcs_target[pajek_arcs_edges_count] = target;
  pajek_arcs_value[pajek_arcs_edges_count] = value;
  snprintf(pajek_arcs_label[pajek_arcs_edges_count],sizeof(char)*16,"%s",label);
  pajek_arcs_width[pajek_arcs_edges_count] = width;
  snprintf(pajek_arcs_colour[pajek_arcs_edges_count],sizeof(char)*16,"%s",colour);
	pajek_arcs_interval_start[pajek_arcs_edges_count] =  start;
  pajek_arcs_interval_end[pajek_arcs_edges_count] =  end;
  pajek_arcs_isEdge[pajek_arcs_edges_count] = isedge;

	pajek_arcs_edges_count++;
	if (isedge){
		pajek_edges_count++;
	} else {
    pajek_arcs_count++;
	}
	return true;
}

/* Add kinds of relation for a multiple edge/arc network.*/
bool pajek_init_KindsOfRelation(char const *relation, bool isedge)
{
  if (max(pajek_arcs_kindsOf_count,pajek_edges_kindsOf_count)< PAJEK_KINDS_RELATIONS){
		if (isedge) {
      snprintf(pajek_edges_kindsOf[pajek_edges_kindsOf_count],sizeof(char)*16,"%s",relation);
			sprintf(pajek_msg,"\nAdded %s relation %i, %s","Edge",pajek_edges_kindsOf_count+1,relation);
			PAJEK_MSG(pajek_msg);
	    pajek_edges_kindsOf_count++;
		} else {
	  	snprintf(pajek_arcs_kindsOf[pajek_arcs_kindsOf_count],sizeof(char)*16,"%s",relation);
      sprintf(pajek_msg,"\nAdded %s relation %i, %s","Arc",pajek_arcs_kindsOf_count+1,relation);
			PAJEK_MSG(pajek_msg);
	    pajek_arcs_kindsOf_count++;
		}
		return true;
	} else {
		PAJEK_MSG("\nErrror! Increase the #define PAJEK_KINDS_RELATIONS.\n");
		return false;
	}
}


/* Initialise the pajek network. */
bool pajek_init( int serial, bool append, char const *dirname_suffix, char const *filename_suffix )
{
	if (!append && PAJEK_FORCE_COMPLETE){
		PAJEK_MSG("\n Error! Append (paj project file) mode necessary in" \
							" combination with  timeline/dynamic modus. Now enforced.");
    pajek_append_mode=true;
	} else {
    pajek_append_mode=append;
	}
  pajek_clear();
  pajek_time=0;
  pajek_snapshot_count=0;
  pajek_snapshot_parts=0;
  pajek_arcs_kindsOf_count= 0; //Reset
	pajek_edges_kindsOf_count = 0;
	if(serial == -1){
  	pajek_serial++;
	} else {
    pajek_serial = serial;
	}

  snprintf(pajek_dirname_suffix,sizeof(char)*196,"%s",dirname_suffix);
  snprintf(pajek_filename_suffix,sizeof(char)*64,"%s",filename_suffix);

 	//Create new or clean existing pajek file
	if (pajek_append_mode){
    if (PAJEK_FORCE_COMPLETE){
	    if (!pajek_init_timeline()){
				return false;
			}
  	}
    PAJEK_MSG("\nTry open in init.");
  	return pajek_openFile(pajek_append_mode, pajek_serial);
	}
	return true;
}

bool pajek_openFile(bool append, int serial, int time)
{
  snprintf(pajek_dir_path,sizeof(char)*64,"%s%s%s",\
			pajek_dirname,\
			FOLDERSEP(pajek_dirname_suffix),pajek_dirname_suffix);
	if (pajek_dirname[0]!='\0')
		NEW_DIR(pajek_dirname);
	if (pajek_dirname_suffix[0]!='\0'){
    NEW_DIR(pajek_dir_path);
	}



	if (!append){
		if (time == -1){
			PAJEK_MSG("\nError: No time provided for non-append call of pajek_openFile");
			return false;
		}
		snprintf(pajek_path,sizeof(char)*300,"%s%s%s_%s_s%d_t%d.%s", \
						pajek_dir_path,\
						FOLDERSEP(pajek_filename),pajek_filename, \
						pajek_filename_suffix, serial, time, "net" );
	} else {
   snprintf(pajek_path,sizeof(char)*300,"%s%s%s_%s_s%d.%s", \
						pajek_dir_path,\
						FOLDERSEP(pajek_filename),pajek_filename, \
						pajek_filename_suffix, serial,  "paj" );
    PAJEK_MSG("\nPath of (temporary) .paj file is:\n\t");
    PAJEK_MSG(pajek_path);
	}
	if (!pajek_file.is_open()){
  	pajek_file.open(pajek_path,std::ios_base::out | std::ios_base::trunc);
		if (pajek_file.is_open()){
			//pajek_file << "\n"; //initalise with a free line.
			return true;
		} else {
			PAJEK_MSG("\npajek_file could not be opened. Check path.");
			return false;
		}
	} else {
		PAJEK_MSG("\npajek_file is already open!.");
		return false;
	}
}
/* Close the file */
bool pajek_close(bool final, bool append)
{
	if (pajek_file.is_open()){
		if (final && append) {
			if (!PAJEK_FORCE_COMPLETE) {
				return pajek_partial_snaps(false); /*do not "reopen" an new file*/
			} else {
				/* Add the time-line info to beginning of the file */
				return pajek_timeline_close();
			}
		} else {
	  	pajek_file.close();
      return true;
		}
	} else {
		PAJEK_MSG("\npajek_file could not be closed for it was not open.");
		return false;
	}
}

/* Save the current snapshot. */
bool pajek_snapshot(int time, bool final)
{
	if (pajek_append_mode){
		return pajek_append (time, final);
	} else {
		return pajek_write(time);
	}
}

/* Write a single static file */
bool pajek_write(int time)
{
	//Add: Control that file is NOT open
  if (time == -1){
		pajek_time++;
	} else{
		pajek_time=time;
	}

	if (!pajek_openFile(false,pajek_serial,pajek_time)){
		PAJEK_MSG("\nCould not open file");
  	return false;
	}

	if (!pajek_append(pajek_time)){
  	PAJEK_MSG("\nCould not append file");
    return false;
	}
	if (!pajek_close()){
    PAJEK_MSG("\nCould not close file");
    return false;
	}

	return true;
}

/* Append to an existing pajek file */
bool pajek_append (int time, bool final )
{
  if (pajek_append_mode){
		pajek_snapshot_count++;
	}
 	if (!pajek_file.is_open()){
		PAJEK_MSG("\npajek_file is not open!");
		return false;
 	}

 //pajek_time is the END of the activity. born is a vertic/edge/arc attribute
 //marking the beginning.
 if (time==-1){
		pajek_time++;
	} else {
		pajek_time=time;
	}
  if (PAJEK_FORCE_COMPLETE){
		//Add time to time-line
    pajek_timeline[pajek_snapshot_count-1]=pajek_time;
 	}


	/*Added also: Static complete structure at beginning of file

	*Network network name
	*Vertices N
	 1 "lab" coordX coordY value shape x_fact factX y_fact factY ic color [activ_int]
	 2 "lab" coordX coordY value shape x_fact factX y_fact factY ic color [activ_int]
	 …
	 N "lab" coordX coordY value shape x_fact factX y_fact factY ic color [activ_int]
	*Arcs :1 "relation name"
	 init_vertex term_vertex value w width c color [activ_int]
	 …
	*Arcs :2 "relation name"
	 init_vertex term_vertex value w width c color [activ_int]
	 …
	…
	*Arcs :R "relation name"
	 init_vertex term_vertex value w width c color [activ_int]
	 …
	*Edges :1 "relation name"
	 init_vertex term_vertex value w width c color [activ_int]
	 …
	*Edges :2 "relation name"
	 init_vertex term_vertex value w width c color [activ_int]
	 …
	…
	*Edges :R "relation name"
	 init_vertex term_vertex value w width c color [activ_int]
	 …

	 */

	//Name network
	/*
    *Network network name in time point T1
	*/
	pajek_file 	<< "*Network " << pajek_filename << "_" \
							<< pajek_filename_suffix << "_s" << pajek_serial;
				if (pajek_time>0)
							pajek_file	<< " in time " << pajek_time << "\n";
				else
							pajek_file << " initial setting\n";
	//Add vertices
	/*
	*Vertices N1
   1 "lab" coordX coordY value shape x_fact factX y_fact factY ic color [activ_int]
	 2 "lab" coordX coordY value shape x_fact factX y_fact factY ic color [activ_int]
	 …
	 N1 "lab" coordX coordY value shape x_fact factX y_fact factY ic color [activ_int]
	*/

	double position_x, position_y;
  pajek_file << "*Vertices " << pajek_vertices_count << "\n";
	for (int i = 0; i < pajek_vertices_count; i++){
    if (PAJEK_FORCE_COMPLETE){
      if(!pajek_vertices_timeline_add(pajek_vertices_ID[i], pajek_vertices_label[i], pajek_snapshot_count)){
      	return false;
			}
		}

  	pajek_relative_xy( (double) (i) / (double) (pajek_vertices_count), \
											 &position_x, &position_y);
		snprintf(	pajek_buffer,sizeof(char)*196, \
						"  %i \"%s\" %g %g %g %s x_fact %g y_fact %g ic %s", \
					 	pajek_vertices_ID[i], /* ID */	\
						pajek_vertices_label[i], /* label */ \
            position_x, /* x-cord */ \
            position_y, /* y-cord  */ \
            pajek_vertices_value[i], /*value*/ \
            pajek_shape(pajek_vertices_shape[i]), /*shape*/ \
            pajek_vertices_xfact[i], /*factX*/ \
            pajek_vertices_yfact[i], /*factY*/ \
            pajek_vertices_colour[i] /*colour*/ \
            );
    pajek_file << pajek_buffer;

	/* Add timestamps / intervals */

		/* Interval option if selected, only applicable if NOT timeline option atm*/
		if (		!PAJEK_FORCE_COMPLETE \
				&& 	pajek_vertices_interval_end[i]!=-1 \
				&& 	pajek_vertices_interval_start[i]!=pajek_vertices_interval_end[i] \
				){

    	pajek_file 	<< " [" << pajek_vertices_interval_start[i] << "-," \
									<< pajek_vertices_interval_end[i] << "]\n";

		/* Timestamps with pot. diff. times per vertice, only if NOT timeline mode*/
		} else if (!PAJEK_FORCE_COMPLETE && pajek_vertices_interval_start[i] != -1){
    	pajek_file << " [" << pajek_vertices_interval_start[i] << "]\n";

		/* Timestamps using uniform time for snapshot */
		} else {
			pajek_file << " [" << pajek_time << "]\n";
		}
	}

	//Add Arcs
	/*
		*Arcs :1 "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
		*Arcs :2 "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
		…
		*Arcs :R "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
	*/
	bool diffKinds = (pajek_arcs_kindsOf_count>0);
	int k = 0;
  int j = 0;
	if (pajek_arcs_count>0){
  	do{
			if (!diffKinds)
				pajek_file << "*Arcs\n";
			else
				pajek_file << "*Arcs :" << k+1 << " \"" << pajek_arcs_kindsOf[k] << "\"\n";
																						//pajek_arcs_label[PAJEK_MAX_ARCS][16]
			for (int i = 0; i < pajek_arcs_edges_count; i++){
				if (!pajek_arcs_isEdge[i] && (!diffKinds || strcmp(pajek_arcs_kindsOf[k],pajek_arcs_label[i])==0) ){

          if (PAJEK_FORCE_COMPLETE){
           	if (!pajek_arcs_timeline_add(pajek_arcs_source[i], pajek_arcs_target[i], pajek_arcs_label[i], pajek_snapshot_count, pajek_arcs_isEdge[i])){
							return false;
						}
					}

          if (diffKinds){
						snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g w %i c %s", \
									 	pajek_arcs_source[i], /* source */	\
										pajek_arcs_target[i], /* target */ \
			              pajek_arcs_value[i], /*value*/ \
			              pajek_arcs_width[i], /*width*/ \
			              pajek_arcs_colour[i] /*colour*/ \
									);
					} else {
            snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g w %i c %s l \"%s\"", \
									 	pajek_arcs_source[i], /* source */	\
										pajek_arcs_target[i], /* target */ \
			              pajek_arcs_value[i], /*value*/ \
			              pajek_arcs_width[i], /*width*/ \
			              pajek_arcs_colour[i], /*colour*/ \
	                  pajek_arcs_label[i] /*label*/\
									);
					}
			    pajek_file << pajek_buffer;

          /* Add timestamps / intervals */

					/* Interval option if selected, only applicable if NOT timeline option atm*/
					if (		!PAJEK_FORCE_COMPLETE \
							&& 	pajek_arcs_interval_end[i]!=-1 \
							&& 	pajek_arcs_interval_start[i]!=pajek_arcs_interval_end[i] \
							){

			    	pajek_file 	<< " [" << pajek_arcs_interval_start[i] << "-," \
												<< pajek_arcs_interval_end[i] << "]\n";

					/* Timestamps with pot. diff. times per vertice, only if NOT timeline mode*/
					} else if (!PAJEK_FORCE_COMPLETE && pajek_arcs_interval_start[i] != -1){
			    	pajek_file << " [" << pajek_arcs_interval_start[i] << "]\n";

					/* Timestamps using uniform time for snapshot */
					} else {
						pajek_file << " [" << pajek_time << "]\n";
					}
					j++;
				}
			}
			k++;
		} while (k<pajek_arcs_kindsOf_count);
		if (j != pajek_arcs_count){
			sprintf(pajek_msg,"\n ERROR pajek arcs count. Is: %i, should: %i." \
					"Check if kinds of relations is correct",j,pajek_arcs_count);
			PAJEK_MSG(pajek_msg);
		}
	}
  //Add Edges
	/*
		*Arcs :1 "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
		*Arcs :2 "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
		…
		*Arcs :R "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
	*/
  diffKinds = (pajek_edges_kindsOf_count>0);
	k = 0;
  j = 0;
	if (pajek_edges_count>0){
  	do{
			if (!diffKinds)
				pajek_file << "*Edges\n";
			else
				pajek_file << "*Edges :" << k+1+pajek_arcs_kindsOf_count << " \"" << pajek_edges_kindsOf[k] << "\"\n";
																						//pajek_arcs_label[PAJEK_MAX_ARCS][16]
			for (int i = 0; i < pajek_arcs_edges_count; i++){
				if (pajek_arcs_isEdge[i] && (!diffKinds || strcmp(pajek_edges_kindsOf[k],pajek_arcs_label[i])==0 ) ){
        	if (PAJEK_FORCE_COMPLETE){
           	if (!pajek_arcs_timeline_add(pajek_arcs_source[i], pajek_arcs_target[i], pajek_arcs_label[i], pajek_snapshot_count, pajek_arcs_isEdge[i])){
            	return false;
						}
					}
     		if (diffKinds){
						snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g w %i c %s", \
									 	pajek_arcs_source[i], /* source */	\
										pajek_arcs_target[i], /* target */ \
			              pajek_arcs_value[i], /*value*/ \
			              pajek_arcs_width[i], /*width*/ \
			              pajek_arcs_colour[i] /*colour*/ \
									);
					} else {
            snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g w %i c %s l \"%s\"", \
									 	pajek_arcs_source[i], /* source */	\
										pajek_arcs_target[i], /* target */ \
			              pajek_arcs_value[i], /*value*/ \
			              pajek_arcs_width[i], /*width*/ \
			              pajek_arcs_colour[i], /*colour*/ \
	                  pajek_arcs_label[i] /*label*/\
									);
					}
			    pajek_file << pajek_buffer;

          /* Add timestamps / intervals */

					/* Interval option if selected, only applicable if NOT timeline option atm*/
					if (		!PAJEK_FORCE_COMPLETE \
							&& 	pajek_arcs_interval_end[i]!=-1 \
							&& 	pajek_arcs_interval_start[i]!=pajek_arcs_interval_end[i] \
							){

			    	pajek_file 	<< " [" << pajek_arcs_interval_start[i] << "-," \
												<< pajek_arcs_interval_end[i] << "]\n";

					/* Timestamps with pot. diff. times per vertice, only if NOT timeline mode*/
					} else if (!PAJEK_FORCE_COMPLETE && pajek_arcs_interval_start[i] != -1){
			    	pajek_file << " [" << pajek_arcs_interval_start[i] << "]\n";

					/* Timestamps using uniform time for snapshot */
					} else {
						pajek_file << " [" << pajek_time << "]\n";
					}
					j++;
				}
			}
			k++;
		} while (k<pajek_edges_kindsOf_count);
		if (j != pajek_edges_count){
			sprintf(pajek_msg,"\n ERROR pajek edges count. Is: %i, should: %i."\
					"Check if kinds of relations is correct",j,pajek_edges_count);
			PAJEK_MSG(pajek_msg);
		}
	}

	if (!final){
		pajek_clear(); //Set counters back for next slice
	} else {
		pajek_close(true,true);
    pajek_write(pajek_time); //Additional writing of file.
		PAJEK_MSG("\nFinal network saved to:\n\t");
		PAJEK_MSG(pajek_path);
	}

	#ifdef PAJEK_SNAPSHOTS_PER_FILE
		if (pajek_append_mode && !PAJEK_FORCE_COMPLETE){
			if ( pajek_snapshot_count - pajek_snapshot_parts*PAJEK_SNAPSHOTS_PER_FILE \
					 >= PAJEK_SNAPSHOTS_PER_FILE ){
        return pajek_partial_snaps(true);
			}
		}
	#endif
	return true;
}

bool pajek_partial_snaps(bool renew)
{
	PAJEK_MSG("\nSaving partial project file.");
  if (pajek_file.is_open()){
  	pajek_file.close();
	} else {
    PAJEK_MSG("\npajek_file could not be closed (here in pajek_partial_snaps).");
		return false;
	}
	char rename[300];
	/*snprintf(pajek_path,sizeof(char)*300,"%s%s%s_%s_%d.%s", \
						pajek_dir_path,\
						FOLDERSEP(pajek_filename),pajek_filename, \
						pajek_filename_suffix, serial, "paj" ); */
  snprintf(rename,sizeof(char)*300,"%s%s%s_%s_s%d", \
						pajek_dir_path,\
						FOLDERSEP(pajek_filename),pajek_filename, \
						pajek_filename_suffix, pajek_serial );
	snprintf(rename,sizeof(char)*300,"%s_part%i.%s",rename,\
				pajek_snapshot_parts+1, "paj");
	if(std::remove(rename)==0){
		PAJEK_MSG("\nDeleted old file with same name.");
	}
	if(std::rename(pajek_path, rename)!=0) {
		PAJEK_MSG("\nError renaming");
		return false;
	} else {
    PAJEK_MSG("\nSaved (renamed) snapshot to: ");
    PAJEK_MSG(rename);
	}
  pajek_snapshot_parts++;
	if (renew){
  	return pajek_openFile(true,pajek_serial);
	} else {
		snprintf(pajek_path,sizeof(char)*300,"%s",rename); //copy the "new" path
		PAJEK_MSG("\nFinal pajek file saved, temporary file removed. Saved to:\n\t");
		PAJEK_MSG(pajek_path);
    pajek_file.close();
		return true;
	}
}

char *pajek_shape(int shape){
	switch (shape){
		case 0: return "ellipse";
		case 1: return "box";
		case 2: return "diamond";
		case 3: return "triangle";
		case 4: return "cross";
		case 5: return "empty";
		case 6: return "house";
		case 7: return "man";
		case 8: return "woman";
		default: return "ellipse";
	}
}

/* Calculate the position on the circle. */
bool pajek_relative_xy(double tau, double *pos_x,double *pos_y)
{
	if (tau > 1.0 || tau < 0.0){
		PAJEK_MSG("\nError in pajek_relative_xy. Invalid value for tau");
		return false;
	}
	*pos_x = .5 * cos(tau*2*3.14159265358979323846) + .5;
	*pos_y = .5 * sin(tau*2*3.14159265358979323846) + .5;
	return true;
}

/* Initialise the memory for the activity overview*/
bool pajek_init_timeline(){
	for (int i = 0; i< PAJEK_MAX_ARCS; i++){
    pajek_arcs_timeStamps_source[i]=-1; //Initial value = not assigned
		for (int j = 0; j < PAJEK_MAX_SNAPSHOTS; j++){
			pajek_arcs_timeStamps[i][j]=false; //Init: Not active
		}
	}
	for (int i = 0; i < PAJEK_MAX_VERTICES; i++){
    pajek_vertices_timeStamps_id[i]=-1; //Initial value = not assigned
	}
	return true;
}

/* Mark vertice with ID add time snapshot as active */
bool pajek_vertices_timeline_add(int ID, char const *label, int snapshot){
	if (snapshot > PAJEK_MAX_SNAPSHOTS){
		PAJEK_MSG("\nError! More snapshots than allowed via initialisation (PAJEK_MAX_SNAPSHOTS)");
		return false;
	}
	int i = 0;
	while (			pajek_vertices_timeStamps_id[i]!=-1 \
					&& 	pajek_vertices_timeStamps_id[i]!= ID ){
		i++;
		if (i == PAJEK_MAX_VERTICES){
			PAJEK_MSG("\nError in pajek_vertices_timeline_add");
			return false;
		}
	}
	if (pajek_vertices_timeStamps_id[i] == -1){
	  pajek_vertices_timeStamps_id[i]=ID;
    snprintf(pajek_vertices_timeStamps_label[i],sizeof(char)*16,"%s",label);
	}
	pajek_vertices_timeStamps[i][snapshot-1]=true;
	return true;
}

/* Mark arcs/edges with Source and Target and Kind add time snapshot as active */
bool pajek_arcs_timeline_add(int Source, int Target, char const *Kind, int snapshot, bool isEdge){
  if (snapshot > PAJEK_MAX_SNAPSHOTS){
		PAJEK_MSG("\nError! More snapshots than allowed via initialisation (PAJEK_MAX_SNAPSHOTS)");
		return false;
	}
	int i = 0;

	/*
	sprintf(pajek_msg,"\nFirst: s%i,t%i,k%i",\
		pajek_arcs_timeStamps_source[i],pajek_arcs_timeStamps_target[i],\
		pajek_arcs_timeStamps_isEdge[i]);
	PAJEK_MSG(pajek_msg);
	*/

	while (			pajek_arcs_timeStamps_source[i]!= -1 \
					&& 	(		pajek_arcs_timeStamps_source[i]!= Source \
							||  pajek_arcs_timeStamps_target[i]!= Target \
							|| 	strcmp(pajek_arcs_timeStamps_kind[i],Kind)!=0 \
							) \
				 ){
		i++;
		/*
		sprintf(pajek_msg,"%i-",i);
		PAJEK_MSG(pajek_msg);
    */
		if (i==PAJEK_MAX_ARCS){
			PAJEK_MSG("\nError in pajek_arcs_timeline_add.");
			return false;
		}
	}
	if (pajek_arcs_timeStamps_source[i] == -1){
	  pajek_arcs_timeStamps_source[i]= Source;
  	pajek_arcs_timeStamps_target[i]= Target;
    pajek_arcs_timeStamps_isEdge[i]= isEdge;
		snprintf(pajek_arcs_timeStamps_kind[i],sizeof(char)*16,"%s",Kind);
	}

	pajek_arcs_timeStamps[i][snapshot-1]=true;
	return true;
}

/* Create a new file for the complete dynamic network. Add the info. Copy the
	content of the old file. Delete the old file. */
bool pajek_timeline_close(){
  char newFilePath[300]; //Final File Name
	/*snprintf(pajek_path,sizeof(char)*300,"%s%s%s_%s_%d.%s", \
						pajek_dir_path,\
						FOLDERSEP(pajek_filename),pajek_filename, \
						pajek_filename_suffix, serial, "paj" ); */
  snprintf(newFilePath,sizeof(char)*300,"%s%s%s_%s_s%d", \
						pajek_dir_path,\
						FOLDERSEP(pajek_filename),pajek_filename, \
						pajek_filename_suffix, pajek_serial );
	char TimesPath[300];
	snprintf(TimesPath,sizeof(char)*300,"%s_time.txt",newFilePath);
  snprintf(newFilePath,sizeof(char)*300,"%s_timeline.%s",newFilePath, "paj");

	/* Close the pajek project file. */
	pajek_file.close();

	/* Open the "new" file for output, the "old" one for input */
  if (!pajek_file.is_open()){
  	pajek_file.open(newFilePath,std::ios_base::out | std::ios_base::trunc);
    if (!pajek_file.is_open() ){
			PAJEK_MSG("\nIn close_timeline() new output pajek_file could not be opened. Check path.");
			return false;
		}
		pajek_file_in.open(pajek_path,std::ios_base::in);
		if (!pajek_file_in.is_open()){
			PAJEK_MSG("\nIn close_timeline() pajek_file_in could not be opened. Check path.");
			return false;
		}
	} else {
		PAJEK_MSG("\nIn close_timeline() pajek_file is still open!.");
		return false;
	}

/* Now, write the summary timeline network first. */

	/* Count number of vertices */
	int n_vert;
	for (n_vert = 0; pajek_vertices_timeStamps_id[n_vert]!=-1 && n_vert < PAJEK_MAX_VERTICES;){
    n_vert++;
	}

	//Name network
	/*
    *Network network name in timeline
	*/
	pajek_file 	<< "*Network " << pajek_filename << "_" \
							<< pajek_filename_suffix << "_s" << pajek_serial << "\n";
	//Add vertices
	/*
	*Vertices N
   1 "lab" coordX coordY value shape x_fact factX y_fact factY ic color [activ_int]
	 2 "lab" coordX coordY value shape x_fact factX y_fact factY ic color [activ_int]
	 …
	 N1 "lab" coordX coordY value shape x_fact factX y_fact factY ic color [activ_int]
	*/

	double position_x, position_y;
	bool first;
  pajek_file << "*Vertices " << n_vert << "\n";
	for (int i = 0; i < n_vert; i++){
  	pajek_relative_xy( (double) (pajek_vertices_timeStamps_id[i]) / (double) (n_vert), \
											 &position_x, &position_y);
		snprintf(	pajek_buffer,sizeof(char)*196, \
						"  %i \"%s\" %g %g %g", \
					 	pajek_vertices_timeStamps_id[i], /* ID */	\
						pajek_vertices_label[i], /* label */ \
            position_x, /* x-cord */ \
            position_y, /* y-cord  */ \
            0.5 /*fake value*/ \
            );
    pajek_file << pajek_buffer;
		pajek_file << " [";  /* if there is an entry then at least
														once there is a snapshot*/
		first = true;
		for (int j=0; j<pajek_snapshot_count;j++){
			if(pajek_vertices_timeStamps[i][j]){
				if (!first){
					pajek_file << ",";
				}
				first = false;
				pajek_file << pajek_timeline[j];
			}
		}
    pajek_file << "]\n";
	}

	//Add Arcs
	/*
		*Arcs :1 "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
		*Arcs :2 "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
		…
		*Arcs :R "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
	*/
	bool diffKinds = (pajek_arcs_kindsOf_count>0);
	int k = 0;
  int j = 0;
	int m_arcs = 0, m=0;
	int n_arcs_edges_total = 0;
	while (m<PAJEK_MAX_ARCS && pajek_arcs_timeStamps_source[m] != -1){
		if (!pajek_arcs_timeStamps_isEdge[m]){
      m_arcs++;
		}
    n_arcs_edges_total++;
		m++;
	}

	if (m_arcs>0){
  	do{
			if (!diffKinds)
				pajek_file << "*Arcs\n";
			else
				pajek_file << "*Arcs :" << k+1 << " \"" << pajek_arcs_kindsOf[k] << "\"\n";
																						//pajek_arcs_label[PAJEK_MAX_ARCS][16]
			for (int i = 0; i < n_arcs_edges_total; i++){
				if (!pajek_arcs_timeStamps_isEdge[i] && (!diffKinds || strcmp(pajek_arcs_kindsOf[k],pajek_arcs_timeStamps_kind[i])==0) ){
          if (diffKinds){
						snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g", \
									 	pajek_arcs_timeStamps_source[i], /* source */	\
										pajek_arcs_timeStamps_target[i], /* target */ \
                 		0.0 /* pseudo value */
									);
					} else {
            snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g l \"%s\"", \
									 	pajek_arcs_timeStamps_source[i], /* source */	\
										pajek_arcs_timeStamps_target[i], /* target */ \
                 		0.0, /* pseudo value */ \
	                  pajek_arcs_timeStamps_kind[i] /*label*/\
									);
					}

          pajek_file << pajek_buffer;
					/* Timestamps */
					pajek_file << " [";  /* if there is an entry then at least
																	once there is a snapshot*/
					first = true;
					for (int l=0; l<pajek_snapshot_count;l++){
						if(pajek_arcs_timeStamps[i][l]){
							if (!first){
								pajek_file << ",";
							}
							first = false;
							pajek_file << pajek_timeline[l];
						}
					}
			    pajek_file << "]\n";
					j++;
				}
			}
			k++;
		} while (k<pajek_arcs_kindsOf_count);
		if (j != m_arcs){
			sprintf(pajek_msg,"\n ERROR in pajek_timeline_close(): pajek arcs count. Is: %i, should: %i." \
					"Check if kinds of relations is correct",j,pajek_arcs_count);
			PAJEK_MSG(pajek_msg);
		}
	}
  //Add Edges
	/*
		*Edges :1 "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
		*Edges :2 "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
		…
		*Edges :R "relation name"
		 init_vertex term_vertex value w width c color [activ_int]
		 …
	*/
  diffKinds = (pajek_edges_kindsOf_count>0);
	k = 0;
  j = 0;
	m = 0;
	int m_edges=0;
	while (m<PAJEK_MAX_ARCS && pajek_arcs_timeStamps_source[m] != -1){
		if (pajek_arcs_timeStamps_isEdge[m]){
			m_edges++;
		}
		m++;
	}

  sprintf(pajek_msg,"\nTotal number of arcs/edges in all time is %i (edges %i, arcs %i)",n_arcs_edges_total,m_edges,m_arcs);
	PAJEK_MSG(pajek_msg);

	if (m_edges>0){
  	do{
			if (!diffKinds)
				pajek_file << "*Edges\n";
			else
				pajek_file << "*Edges :" << k+1+pajek_arcs_kindsOf_count << " \"" << pajek_edges_kindsOf[k] << "\"\n";
																						//pajek_arcs_label[PAJEK_MAX_ARCS][16]
   for (int i = 0; i < n_arcs_edges_total; i++){
				if (pajek_arcs_timeStamps_isEdge[i] && (!diffKinds || strcmp(pajek_edges_kindsOf[k],pajek_arcs_timeStamps_kind[i])==0) ){
          if (diffKinds){
						snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g", \
									 	pajek_arcs_timeStamps_source[i], /* source */	\
										pajek_arcs_timeStamps_target[i], /* target */ \
                 		0.0 /* pseudo value */
									);
					} else {
            snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g l \"%s\"", \
									 	pajek_arcs_timeStamps_source[i], /* source */	\
										pajek_arcs_timeStamps_target[i], /* target */ \
                 		0.0, /* pseudo value */ \
	                  pajek_arcs_timeStamps_kind[i] /*label*/\
									);
					}
			    pajek_file << pajek_buffer;
					/* Timestamps */
					pajek_file << " [";  /* if there is an entry then at least
																	once there is a snapshot*/
					first = true;
     			for (int l=0; l<pajek_snapshot_count;l++){
						if(pajek_arcs_timeStamps[i][l]){
							if (!first){
								pajek_file << ",";
							}
							first = false;
							pajek_file << pajek_timeline[l];
						}
					}
			    pajek_file << "]\n";
					j++;
				}
			}
			k++;
		} while (k<pajek_edges_kindsOf_count);
		if (j != m_edges){
			sprintf(pajek_msg,"\n  ERROR in pajek_timeline_close(): pajek edges count. Is: %i, should: %i."\
					"Check if kinds of relations is correct",j,pajek_edges_count);
			PAJEK_MSG(pajek_msg);
		}
	}

	/* Now that the "new" file is fine, add the content of the former snapshots
		file */

	std::string str;
  while(std::getline(pajek_file_in,str)){
 		pajek_file<<str<<endl;
	}

	/* Close files and delete old pajek file */
	pajek_file.close();
  pajek_file_in.close();
 	std::remove(pajek_path);

 /* Create "Times" file */
 	pajek_file.open(TimesPath,std::ios_base::out | std::ios_base::trunc);
	for (int i = 0; i < pajek_snapshot_count; i++){
		pajek_file << i+1 << "\t";
		if (i==0){
      pajek_file << "Initial set-up\n";
		} else if (i < pajek_snapshot_count-1){
    	pajek_file <<  "At time " << pajek_timeline[i]  <<"\n";
		} else {
    	pajek_file <<  "At time " << pajek_timeline[i]  << " (final)" << "\n";
		}
	}
  pajek_file.close();


	return true;

}
