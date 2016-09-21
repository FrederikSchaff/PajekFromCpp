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
bool pajek_vertices_add(int ID, char const label[],  int shape, char const *colour, double x_fact, double y_fact, double value, double x_pos, double y_pos, int start, int end )
{

	if (pajek_vertices_count == PAJEK_MAX_VERTICES) {
    PAJEK_MSG("\nERROR! Maximum number of vertices reached");
		return false;
	}

  pajek_vertices_ID[pajek_vertices_count]	= ID;
  snprintf(pajek_vertices_label[pajek_vertices_count],sizeof(char)*PAJEK_LABELSIZE,"%s",label); //Check
  pajek_vertices_shape[pajek_vertices_count] = shape;
	//0=ellipse, 1=box, 2=diamond, 3=triangle, 4=cross, 5=empty, 6=house, 7=man, or 8=woman
  snprintf(pajek_vertices_colour[pajek_vertices_count],sizeof(char)*16,"%s",colour);
  pajek_vertices_xfact[pajek_vertices_count] = x_fact;
  pajek_vertices_yfact[pajek_vertices_count] = y_fact>0?y_fact:x_fact;
  pajek_vertices_value[pajek_vertices_count] = value;
	pajek_vertices_interval_start[pajek_vertices_count] = start;
  pajek_vertices_interval_end[pajek_vertices_count] = end;
  pajek_vertices_x_pos[pajek_vertices_count] = x_pos;
  pajek_vertices_y_pos[pajek_vertices_count] = y_pos;

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
  snprintf(pajek_arcs_label[pajek_arcs_edges_count],sizeof(char)*PAJEK_LABELSIZE,"%s",label);
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
  if (pajek_init_KindsOfRelation_check){
    if (max(pajek_arcs_kindsOf_count,pajek_edges_kindsOf_count)< PAJEK_KINDS_RELATIONS){
  		if (isedge) {
        snprintf(pajek_edges_kindsOf[pajek_edges_kindsOf_count],sizeof(char)*PAJEK_LABELSIZE,"%s",relation);
  			sprintf(pajek_msg,"\nAdded %s relation %i, %s","Edge",pajek_edges_kindsOf_count+1,relation);
  			PAJEK_MSG(pajek_msg);
  	    pajek_edges_kindsOf_count++;
  		} else {
  	  	snprintf(pajek_arcs_kindsOf[pajek_arcs_kindsOf_count],sizeof(char)*PAJEK_LABELSIZE,"%s",relation);
        sprintf(pajek_msg,"\nAdded %s relation %i, %s","Arc",pajek_arcs_kindsOf_count+1,relation);
  			PAJEK_MSG(pajek_msg);
  	    pajek_arcs_kindsOf_count++;
  		}
  	  pajek_init_KindsOfRelation_check=false;
    	return true;
  	} else {
  		PAJEK_MSG("\nErrror! Increase the #define PAJEK_KINDS_RELATIONS.\n");
  		return false;
  	}
  } else {
    PAJEK_MSG("\nError! First initialise pajek via pajek_init()!");
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
  pajek_init_KindsOfRelation_check=true; //Enable initialisation of kinds of rel.
  pajek_clear();
  pajek_vertice_xy_pos_numb = -1; /* Not yet analysed */
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
			pajek_dirname,  /*Ensure that there is no "/" at the end*/ \
			FOLDERSEP2(pajek_dirname, pajek_dirname_suffix) \
			,pajek_dirname_suffix);
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
						FOLDERSEP(pajek_dir_path),pajek_filename, \
						pajek_filename_suffix, serial, time, "net" );
	} else {
   snprintf(pajek_path,sizeof(char)*300,"%s%s%s_%s_s%d.%s", \
						pajek_dir_path,\
						FOLDERSEP(pajek_dir_path),pajek_filename, \
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
	int maxID;
  if (PAJEK_FORCE_COMPLETE){
	  /* Ensure increasing order of and consecutive ids in time-line mode */
	  maxID=pajek_consistent_IDs();
	}

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

  int id;
	char label[PAJEK_LABELSIZE+6];
	double position_x, position_y;
  pajek_file << "*Vertices " << pajek_vertices_count << "\n";
	for (int i = 0; i < pajek_vertices_count; i++){
    if (PAJEK_UNIQUE_VERTICE_LABELS){
  		snprintf(label,sizeof(char)*(PAJEK_LABELSIZE),"%s", \
				pajek_vertices_label[i]);
		} else {
      snprintf(label,sizeof(char)*(PAJEK_LABELSIZE+6),"%s_%05d", \
				pajek_vertices_label[i],pajek_vertices_ID[i]);
		}

    position_x=pajek_vertices_x_pos[i];
    position_y=pajek_vertices_y_pos[i];
		if (position_x < 0 || position_x > 1 || position_y < 0 || position_y > 1){
      if (PAJEK_FORCE_COMPLETE){
      	pajek_relative_xy( (double) (pajek_vertices_ID[i]-1) / (double) (maxID), \
												 &position_x, &position_y);
			} else {
        pajek_relative_xy( (double) (i) / (double) (pajek_vertices_count), \
											 &position_x, &position_y);
			}
		}

    if (PAJEK_FORCE_COMPLETE){
      id=pajek_Unique2Consequtive[pajek_vertices_ID[i]]; //ensures "sorted" IDs 	
      if(!pajek_vertices_timeline_add(id, pajek_vertices_label[i], pajek_snapshot_count)){
      	return false;
			}
		} else{
      id=pajek_vertices_ID[i]; //The vertices may not be sorted by id.
    }

		snprintf(	pajek_buffer,sizeof(char)*196, \
						"  %i \"%s\" %g %g %g %s x_fact %g y_fact %g ic %s", \
					 	id, /* ID */	\
						label, /* label */ \
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
	if (pajek_arcs_edges_count == 0){
    PAJEK_MSG("\nNo Edges or Arcs! (Leads to errors with PajekToSvgAnim)");
	}
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
	int source,target;
	if (pajek_arcs_count>0){
  	do{
			if (!diffKinds)
				pajek_file << "*Arcs\n";
			else
				pajek_file << "*Arcs :" << k+1 << " \"" << pajek_arcs_kindsOf[k] << "\"\n";

			for (int i = 0; i < pajek_arcs_edges_count; i++){
				if (!pajek_arcs_isEdge[i] && (!diffKinds || strncmp(pajek_arcs_kindsOf[k],pajek_arcs_label[i],PAJEK_LABELSIZE)==0) ){

          if (PAJEK_FORCE_COMPLETE){
          	source = pajek_Unique2Consequtive[pajek_arcs_source[i]];
						target = pajek_Unique2Consequtive[pajek_arcs_target[i]];
           	if (!pajek_arcs_timeline_add(source, target, pajek_arcs_label[i], pajek_snapshot_count, pajek_arcs_isEdge[i])){
							return false;
						}
					} else {
						source = pajek_arcs_source[i];
						target = pajek_arcs_target[i];
					}
          if (diffKinds){
						snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g w %i c %s", \
									 	source, /* source */	\
										target, /* target */ \
			              pajek_arcs_value[i], /*value*/ \
			              pajek_arcs_width[i], /*width*/ \
			              pajek_arcs_colour[i] /*colour*/ \
									);
					} else {
            snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g w %i c %s l \"%s\"", \
									 	source, /* source */	\
										target, /* target */ \
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
	if (pajek_edges_count>0){
  	do{
			if (!diffKinds)
				pajek_file << "*Edges\n";
			else
				pajek_file << "*Edges :" << k+1+pajek_arcs_kindsOf_count << " \"" << pajek_edges_kindsOf[k] << "\"\n";

			for (int i = 0; i < pajek_arcs_edges_count; i++){
				if (pajek_arcs_isEdge[i] && (!diffKinds || strncmp(pajek_edges_kindsOf[k],pajek_arcs_label[i],PAJEK_LABELSIZE)==0 ) ){
        	if (PAJEK_FORCE_COMPLETE){
          	source = pajek_Unique2Consequtive[pajek_arcs_source[i]];
						target = pajek_Unique2Consequtive[pajek_arcs_target[i]];
           	if (!pajek_arcs_timeline_add(source, target, pajek_arcs_label[i], pajek_snapshot_count, pajek_arcs_isEdge[i])){
            	return false;
						}
					} else {
						source = pajek_arcs_source[i];
						target = pajek_arcs_target[i];
					}
     		if (diffKinds){
						snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g w %i c %s", \
									 	source, /* source */	\
										target, /* target */ \
			              pajek_arcs_value[i], /*value*/ \
			              pajek_arcs_width[i], /*width*/ \
			              pajek_arcs_colour[i] /*colour*/ \
									);
					} else {
            snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g w %i c %s l \"%s\"", \
									 	source, /* source */	\
										target, /* target */ \
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
	/*Add a blank line*/
  pajek_file << "\n";

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

  snprintf(rename,sizeof(char)*300,"%s%s%s_%s_s%d", \
						pajek_dir_path,\
						FOLDERSEP(pajek_dir_path),pajek_filename, \
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

char const *pajek_shape(int shape){
	if (PAJEK_FORCE_COMPLETE){
		/* PajekToSvgAnim does not support man/woman shape!*/
		if (shape == 7){
			shape = 2;
		} else if (shape == 8) {
			shape = 3;
		}
	}
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
bool pajek_relative_xy(double tau, double *pos_x, double *pos_y, double radius, double x_orig, double y_orig)
{
	if (tau > 1.0 || tau < 0.0){
		PAJEK_MSG("\nError in pajek_relative_xy. Invalid value for tau");
		return false;
	}

	*pos_x = radius * cos(tau*2*3.14159265358979323846) + x_orig;
	*pos_y = radius * sin(tau*2*3.14159265358979323846) + y_orig;

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
    for (int j = 0; j < PAJEK_MAX_SNAPSHOTS; j++){
    	pajek_vertices_timeStamps[i][j]=false;  //Init: Not active
		}
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
		if (PAJEK_UNIQUE_VERTICE_LABELS){
    	snprintf(pajek_vertices_timeStamps_label[i], \
				sizeof(char)*(PAJEK_LABELSIZE),"%s",label);
		} else {
    	snprintf(pajek_vertices_timeStamps_label[i], \
				sizeof(char)*(PAJEK_LABELSIZE+6),"%s_%05d",label,ID);
		}
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
							|| 	strncmp(pajek_arcs_timeStamps_kind[i],Kind,PAJEK_LABELSIZE)!=0 \
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
		snprintf(pajek_arcs_timeStamps_kind[i],sizeof(char)*PAJEK_LABELSIZE,"%s",Kind);
	}

	pajek_arcs_timeStamps[i][snapshot-1]=true;
	return true;
}


/* Create a new file for the complete dynamic network. Add the info. Copy the
	content of the old file. Delete the old file.
	For PajekToSvg some things need to hold for the *new* first slice:
	-	For each ID from 1 to the maximum, a vertice needs to exist.
	- These vertices need to be printed in increasing order from top to down
	- This needs to be taken care of for EACH snapshot
	- For each vertice that is ever active, all active time-stamps need be present
	- Each arc that will ever be created must be provided with all time-stamps
		where it exists
	- if multiple relations are chosen, the counter for the relations needs to be
		consequtive accross the relations.
	All this is taken care of.
	*/
bool pajek_timeline_close(){
  char newFilePath[300]; //Final File Name

  snprintf(newFilePath,sizeof(char)*300,"%s%s%s_%s_s%d", \
						pajek_dir_path,\
						FOLDERSEP(pajek_dir_path),pajek_filename, \
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

/* Now, write the summary timeline network first.
	Ensure increasing order of ids*/

  int maxID = 0;
	int n_vertices=0;
	for (int i = 0; pajek_vertices_timeStamps_id[i]!=-1 && i <PAJEK_MAX_VERTICES;\
				i++,n_vertices++){
    pajek_Consequtive2Unique[i+1]=pajek_vertices_timeStamps_id[i];
    pajek_Unique2Consequtive[pajek_vertices_timeStamps_id[i]]=i+1;
		maxID = max(maxID,pajek_vertices_timeStamps_id[i]);
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
  pajek_file << "*Vertices " << n_vertices << "\n";
	for (int i = 0; i < n_vertices; i++){
    //position_x=pajek_vertices_x_pos[i];
    //position_y=pajek_vertices_y_pos[i];
		//if (position_x < 0 || position_x > 1 || position_y < 0 || position_y > 1){
  		pajek_relative_xy( (double) (i) / (double) (n_vertices), \
											 &position_x, &position_y);
		//}

		snprintf(	pajek_buffer,sizeof(char)*196, \
						"  %i \"%s\" %g %g %g", \
					 	pajek_Unique2Consequtive[pajek_vertices_timeStamps_id[i]], /* Conesqutive ID */	\
						pajek_vertices_timeStamps_label[i], /* label */ \
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
	int source,target;
	int k = 0;  /* Relations counter */
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

			for (int i = 0; i < n_arcs_edges_total; i++){
				if (!pajek_arcs_timeStamps_isEdge[i] && (!diffKinds || strncmp(pajek_arcs_kindsOf[k],pajek_arcs_timeStamps_kind[i],PAJEK_LABELSIZE)==0) ){
          source = pajek_Unique2Consequtive[pajek_arcs_timeStamps_source[i]];
					target = pajek_Unique2Consequtive[pajek_arcs_timeStamps_target[i]];
					if (diffKinds){
						snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g", \
									 	source, /* source */	\
										target, /* target */ \
                 		0.0 /* pseudo value */
									);
					} else {
            snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g l \"%s\"", \
									 	source, /* source */	\
										target, /* target */ \
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

   for (int i = 0; i < n_arcs_edges_total; i++){
				if (pajek_arcs_timeStamps_isEdge[i] && (!diffKinds || strncmp(pajek_edges_kindsOf[k],pajek_arcs_timeStamps_kind[i],PAJEK_LABELSIZE)==0) ){
          source = pajek_Unique2Consequtive[pajek_arcs_timeStamps_source[i]];
					target = pajek_Unique2Consequtive[pajek_arcs_timeStamps_target[i]];
					if (diffKinds){
						snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g", \
									 	source, /* source */	\
										target, /* target */ \
                 		0.0 /* pseudo value */
									);
					} else {
            snprintf(	pajek_buffer,sizeof(char)*196, \
										"  %i %i %g l \"%s\"", \
									 	source, /* source */	\
										target, /* target */ \
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

	/*Add a blank line*/
  pajek_file << "\n";

	/* Now that the "new" file is fine, add the content of the former snapshots
		file */

	std::string str;
  while(std::getline(pajek_file_in,str)){
 		pajek_file<<str<<endl;
	}
  /*
    The following two lines set getline back to the start. 
    See: http://stackoverflow.com/a/5343199
  */
  pajek_file_in.clear();
  pajek_file_in.seekg (0, ios::beg);

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

/* Link the unique, time-consistent IDs to snapshot-unique, consecutive IDs
	Note: unique IDs range from 1 to pajek_vertices_count inclusive!*/
int pajek_consistent_IDs(){
	int maxID = 0;
	for (int i = 0; i < pajek_vertices_count; i++){
    pajek_Consequtive2Unique[i+1]=pajek_vertices_ID[i];
    pajek_Unique2Consequtive[pajek_vertices_ID[i]]=i+1;
		maxID = max(maxID,pajek_vertices_ID[i]);
	}
	return maxID;
}

/* Partition a unit square into >=n non-adjacent squares and report center
	positions and radius for the first n relevant partitions, starting with n=0.
	An odd number of partitions is ensured, i.e. there is always a center square.

	in: number of vertices. Optional: origin (center point of square) and size of
																		overall plane.
	out: radius of circles that can be drawn around each square, useful for
			positioning additional vertices (of another type, say consumers) around
			the original one (say, company).

	manipulates the global variable  pajek_vertice_xy_pos[][2] which may be
		accessed for the position of the single vertices direcly, or (prefered)
		via the functions pajek_vertice_x_pos and pajek_vertice_y_pos
*/
double pajek_partition_unitSquare(int numb, \
									 double orig_x, double orig_y, double orig_size){

	/* Set the number of seperate vertices that shall be present */
  pajek_vertice_xy_pos_numb=numb;

	double diameter = orig_size; //The diameter of the subboxes fitting in each square of the matrix
	//make sure that n is odd.
	int n = numb;
	if (n%2==2){
		n+=1;
	}

	int m=1;
	int rc = 1;
	for (int i = 1; m < n; i++){
		m+=i*4; /*for each side of the cube*/
		rc+=2; /*extend the overal cube by one column/row to left/right/top/down*/
	}
	diameter/=((double)rc );

	pajek_vertice_xy_pos_radius = diameter/2.0; /* Might be useful for "sub" positioning */

	/* Next we draw, starting from the global center,
			the center-positions of the circles. */

  double x_pos=orig_x, y_pos=orig_y;
	pajek_vertice_xy_pos[0][0]=x_pos; //x
  pajek_vertice_xy_pos[0][1]=y_pos; //y

	int doWhat = 5; /* jump, upper, left, lower, right */
	int id = 1;  /* Here, the id ranges from 0 to numb-1 */
	int i = 0;

	while ( id < numb){

		if (doWhat == 5){
    	/* jump to next box */
			x_pos += diameter;
			y_pos += diameter;
			doWhat = 1;
			i++;
		}

    for (int j = 0; j < i; j++){
	    switch(doWhat){

				case 1: /* Draw UPPER points */
					x_pos -= 2*diameter;
					break;

				case 2: /* Draw LEFT points */
			    y_pos -= 2*diameter;
					break;

		    case 3: /* Draw LOWER points */
			    x_pos += 2*diameter;
					break;

				case 4: /* Draw RIGHT points */
			    y_pos += 2*diameter;
					break;
			}

			pajek_vertice_xy_pos[id][0]= x_pos;
	    pajek_vertice_xy_pos[id][1]= y_pos;
			id++;
		}
		doWhat++;
	}

  return diameter/2.0;
}

/* Provide the position for the id ranging from 1 to numb */
double pajek_vertice_x_pos(int id){
	if (id < 1 || id > pajek_vertice_xy_pos_numb) {
		return -1.0; // default value for NOT considering the given pos.
	} else {
  	return pajek_vertice_xy_pos[id-1][0];
	}
}
double pajek_vertice_y_pos(int id){
	if (id < 1 || id > pajek_vertice_xy_pos_numb) {
		return -1.0; // default value for NOT considering the given pos.
	} else {
  	return pajek_vertice_xy_pos[id-1][1];
	}
}
