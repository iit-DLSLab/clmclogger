/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <termio.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <clmclogger/mrdplot.h>

#include <iostream>
#include <fstream>

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

bool MRDPlot::setChannelNameAndUnit(const std::string &n, const std::string &u, size_t idx)
{
  if (n.find(" ") != std::string::npos || n.find("\n") != std::string::npos ||
      u.find(" ") != std::string::npos || u.find("\n") != std::string::npos)
    return false;
  if (idx >= _n_channels)
    return false;

  _channelNames[idx] = n;
  _channelUnits[idx] = u;
  return true;
}

void MRDPlot::alloc(size_t n_ch, size_t n_pt)
{
  if (n_ch == 0 || n_pt == 0)
    return;
  
  _name = "noname";
  _n_points = n_pt;
  _n_channels = n_ch;
  _freq = 0;
  
  if (_n_channels == 0)
    return;

  _channelNames.resize(_n_channels);
  _channelUnits.resize(_n_channels);

  _data.resize(_n_channels*_n_points);
}

bool MRDPlot::readFromFile(const std::string &name)
{
  std::ifstream in;
  in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  
  size_t tot;

  try {
    in.open(name);
    // read header
    in >> tot;
    in >> _n_channels;
    in >> _n_points;
    in >> _freq;

    alloc(_n_channels, _n_points);
    _name = name;
    
    // read the channel names and units
    std::string tmp;
    for (size_t i = 0; i < _n_channels; i++) {
      in >> tmp;
      _channelNames[i] = tmp;
      in >> tmp;
      _channelUnits[i] = tmp;
    }

    // get 3 \n
    in.get();
    in.get();
    in.get();

    // read data
    char *ptr = (char *)&_data[0];
    for (size_t i = 0; i < _n_points*_n_channels; i++) {
      ptr[3] = in.get();
      ptr[2] = in.get();
      ptr[1] = in.get();
      ptr[0] = in.get();

      ptr +=4;
    }
    
    in.get();
  }
  catch (std::ifstream::failure e) {
    std::cerr << "error when parsing data\n";
    return false;
  }

  return true;
}

int MRDPlot::findChannel(const std::string &name) const
{
  for (size_t i = 0; i < _channelNames.size(); i++)
    if (_channelNames[i].compare(name) == 0)
      return i;

  return -1;
}

bool MRDPlot::writeToFile(const std::string &name)
{
  _name = name;
  
  std::ofstream out;
  out.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  
  try {
    out.open(name.c_str(), std::ofstream::out);
    // write header
    out << _n_points*_n_channels << " " << _n_channels << " " << _n_points << " " << _freq << std::endl;

    // write names and units
    for (size_t i = 0; i < _n_channels; i++) {
      out << _channelNames[i] << " " << _channelUnits[i] << std::endl;
    }

    // write 2 empty line
    out << std::endl << std::endl;

    // write data
    for (size_t i = 0; i < _n_points*_n_channels; i++) {
      char *ptr = (char *)(&_data[i]);
      out.put(ptr[3]);
      out.put(ptr[2]);
      out.put(ptr[1]);
      out.put(ptr[0]);
    }

    out.close();
  }
  catch (std::ofstream::failure e) {
    std::cerr << e.what() <<"error when writing data\n";
  }

}




	MRDPLOT_DATA *
malloc_mrdplot_data( int n_channels, int n_points )
{
	MRDPLOT_DATA *d;

	d = (MRDPLOT_DATA *) malloc( sizeof( MRDPLOT_DATA ) );
	if ( d == NULL )
	{
		fprintf( stderr, "Couldn't allocate MRDPLOT_DATA.\n" );
		exit( -1 );
	}
	d->filename = NULL;
	d->n_channels = n_channels;
	d->n_points = n_points;
	d->total_n_numbers = n_channels*n_points;
	d->frequency = 0;
	if ( n_channels > 0 )
	{
		d->names = (char **) malloc( d->n_channels*sizeof(char *) );
		d->units = (char **) malloc( d->n_channels*sizeof(char *) );
		if ( d->names == NULL || d->units == NULL )
		{
			fprintf( stderr,
					"malloc_mrdplot_data: Can't allocate memory for names or units.\n" );
			exit( -1 );
		}
	}
	else
	{
		d->names = NULL;
		d->units = NULL;
	}
	if ( d->total_n_numbers > 0 )
	{
		d->data = (float *) malloc( d->total_n_numbers*sizeof( float ) );
		if ( d->data == NULL )
		{
			fprintf( stderr,
					"malloc_mrdplot_data: Couldn't allocate memory of size %d\n", 
					d->total_n_numbers );
			exit( -1 );
		}
	}
	else
		d->data = NULL;
	return d;
}

/*****************************************************************************/

	MRDPLOT_DATA *
read_mrdplot(const char *filename )
{
	FILE *stream;
	int total_n_numbers, n_channels, n_points;
	float frequency;
	MRDPLOT_DATA *d;
	int i;
	char buffer1[1000];
	char buffer2[1000];
	char *p;
	int n_bytes;

	/* Windows needs binary flag. No effect on Unix */
	stream = fopen( filename, "rb" );
	if ( stream == NULL )
	{
		fprintf( stderr, "Couldn't open %s file for read.\n", 
				filename );
		exit( -1 );
	}

	if ( fscanf( stream, "%d%d%d%f",
				&total_n_numbers, 
				&n_channels, 
				&n_points, 
				&frequency ) != 4 )
	{
		fprintf( stderr, "Header error reading %s\n", filename );
		exit( -1 );
	}

	d = malloc_mrdplot_data( n_channels, n_points );
	d->filename = filename;
	d->frequency = frequency;

	printf(
			"%d points, %d channels in sample, %d numbers total, %g samples/second.\n",
			d->n_points, d->n_channels, 
			d->total_n_numbers, d->frequency );


	for( i = 0; i < d->n_channels; i++ )
	{
		if (fscanf( stream, "%s%s", buffer1, buffer2 ) != 2) {
			fprintf( stderr, "Name parsing error reading %s at channel %d\n", filename, i );
			exit( -1 );
		}
		d->names[i] = strdup( buffer1 );
		d->units[i] = strdup( buffer2 );
		//printf( "%d: %s %s\n", i, d->names[i], d->units[i] );
	}
	int ret = fscanf( stream, "%c%c%c", buffer1, buffer1, buffer1 );
	assert(ret);

	/* SGI version */
	/*
		 fread( d, n_channels*sizeof( float ), n_points, stream );
		 */
	/* Linux version */
	p = (char *) (d->data);
	n_bytes = d->total_n_numbers*4;
	for( i = 0; i < n_bytes; i += 4 )
	{
		ret = fread( &(p[i+3]), 1, 1, stream );
		assert( ret );
		ret = fread( &(p[i+2]), 1, 1, stream ); 
		assert( ret );
		ret = fread( &(p[i+1]), 1, 1, stream );
		assert( ret );
		ret = fread( &(p[i+0]), 1, 1, stream );
		assert( ret );
	}

	fclose( stream );
	return d;
}

/*****************************************************************************/

	int
find_channel( const char *name, MRDPLOT_DATA *d )
{
	int i;

	for ( i = 0; i < d->n_channels; i++ )
	{
		if ( strcmp( name, d->names[i] ) == 0 )
		{
			//printf( "Found %s at %d\n", name, i );
			return i;
		}
	}
	printf( "Didn't find %s\n", name );
	return -1;
}

/*****************************************************************************/
/*****************************************************************************/

void fwrite_reversed( char *p, int i1, int i2, FILE *stream )
{
	int total, i;

	total = i1*i2;

	for( i = 0; i < total; i += 4 )
	{
		fwrite( &(p[i+3]), 1, 1, stream );
		fwrite( &(p[i+2]), 1, 1, stream );
		fwrite( &(p[i+1]), 1, 1, stream );
		fwrite( &(p[i+0]), 1, 1, stream );
	}
}

/*****************************************************************************/

	void
write_mrdplot_file( MRDPLOT_DATA *d )
{
	FILE *stream;
	int i;

	// printf( "Writing %s\n", d->filename );

	/* Windows needs binary flag. No effect on Unix */
	stream = fopen( d->filename, "wb" );
	if ( stream == NULL )
	{
		fprintf( stderr, "Couldn't open %s file for write.\n", d->filename );
		exit( -1 );
	}

	/*
		 printf( "%d %d %d %f\n",
		 d->total_n_numbers, d->n_channels, d->n_points, d->frequency );
		 */

	fprintf( stream, "%d %d %d %f\n",
			d->total_n_numbers, d->n_channels, d->n_points, d->frequency );

	for( i = 0; i < d->n_channels; i++ )
	{
		fprintf( stream, "%s %s\n", d->names[i], d->units[i] );
	}
	// Linux version
	fprintf( stream, "\n\n" );
	// Windows version needs this?
	// fprintf( stream, "\r\r" );


	for( i = 0; i < d->n_points; i++ )
	{
		/* SGI version
			 fwrite( &(data[i*N_CHANNELS]),
			 N_CHANNELS*sizeof( float ), 1, stream );
			 */
		/* Linux version */
		fwrite_reversed( (char *) (&(d->data[i*d->n_channels])),
				d->n_channels*sizeof( float ), 1, stream );
		/*
			 fwrite( (char *) (&(d->data[i*d->n_channels])),
			 d->n_channels*sizeof( float ), 1, stream );
			 */
	}

	fclose( stream );
}

	void
write_mrdplot_file( std::string file_name, int total_n_numbers, int n_channels, int n_points, float frequency,
			float *data, std::vector<std::string> names,std::vector<std::string> units)
{

	FILE *stream;
	int i;

	// printf( "Writing %s\n", d->filename );

	/* Windows needs binary flag. No effect on Unix */
	stream = fopen( file_name.c_str(), "wb" );
	if ( stream == NULL )
	{
		fprintf( stderr, "Couldn't open %s file for write.\n", file_name.c_str() );
		exit( -1 );
	}

	/*
		 printf( "%d %d %d %f\n",
		 d->total_n_numbers, d->n_channels, d->n_points, d->frequency );
		 */

	fprintf( stream, "%d %d %d %f\n",
			total_n_numbers, n_channels, n_points, frequency );

	for( i = 0; i < n_channels; i++ )
	{
		fprintf( stream, "%s %s\n", names[i].c_str(), units[i].c_str() );
		//std::cout << "i " << i << " name " << names[i] << " units " << units[i] << std::endl;
	}
	// Linux version
	fprintf( stream, "\n\n" );
	// Windows version needs this?
	// fprintf( stream, "\r\r" );


	for( i = 0; i < n_points; i++ )
	{
		/* SGI version
			 fwrite( &(data[i*N_CHANNELS]),
			 N_CHANNELS*sizeof( float ), 1, stream );
			 */
		/* Linux version */
		fwrite_reversed( (char *) (&(data[i*n_channels])),
				n_channels*sizeof( float ), 1, stream );
		/*
			 fwrite( (char *) (&(d->data[i*d->n_channels])),
			 d->n_channels*sizeof( float ), 1, stream );
			 */
	}

	fclose( stream );
}

/*****************************************************************************/

char generated_file_name[10000];

/* char *
	 /generate_file_name()
	 {
	 FILE *stream;
	 int file_number;


	 stream = fopen( "last_data", "r" );
	 if ( stream == NULL )
	 file_number = 0;
	 else
	 {
	 if(fscanf( stream, "%d", &file_number ) == 0)
	 {
	 file_number = 0;
	 }
	 fclose( stream );
	 }

	 sprintf( generated_file_name, "d%05d", file_number );

	 file_number++;

	 stream = fopen( "last_data", "w" );
	 if ( stream == NULL )
	 return strdup(generated_file_name);
	 fprintf( stream, "%d\n", file_number );
	 fclose( stream );
	 return strdup(generated_file_name);
	 }
	 */
	char *
generate_file_name(const char *prefix)
{
	FILE *stream;
	char tmp_string_time[1000];
	char tmp_string_user[1000];
	int return_nowarn;
	int file_number = 0;
	time_t now = time(0);
	struct tm tstruct;

	stream = fopen( "/logs/last_data", "r" );
	if ( stream != NULL )
	{
		return_nowarn = fscanf( stream, "%d", &file_number );
		fclose( stream );
	}

	tstruct = *localtime(&now);
	strftime(tmp_string_time, sizeof(tmp_string_time), "%m_%d_%H_%M_%S", &tstruct);

	int i = getlogin_r(tmp_string_user, sizeof(tmp_string_user));
	// getlogin_r seems to be failing in some cases
	if (i != 0)
	{
		tmp_string_user[0] = 0;
		tmp_string_user[1] = 0;
	}


	sprintf(generated_file_name, "/logs/mrdplot/%s_%s_%s.mrd", prefix, tmp_string_user, tmp_string_time);

	stream = fopen( "/logs/last_data", "w" );
	// change the mode of the file so everyone can read/write to it
	// If not owner this will cause warning:
	// chmod: changing permissions of `/logs/last_data': Operation not permitted
	return_nowarn = system("chmod 666 /logs/last_data");
	if ( stream != NULL )
	{
		file_number++;
		fprintf( stream, "%d\n", file_number );
		fclose( stream );
	}

	return strdup(generated_file_name);
}

std::string generate_file_name(std::string prefix)
{
	std::stringstream file_name_ss;
	char tmp_string_time[1000];
	char tmp_string_user[1000];
	time_t now = time(0);
	struct tm tstruct;


	tstruct = *localtime(&now);
	strftime(tmp_string_time, sizeof(tmp_string_time), "%m_%d_%H_%M_%S", &tstruct);

	int i = getlogin_r(tmp_string_user, sizeof(tmp_string_user));
	// getlogin_r seems to be failing in some cases
	if (i != 0)
	{
		tmp_string_user[0] = 0;
		tmp_string_user[1] = 0;
	}

	file_name_ss << "/logs/mrdplot/" << prefix << "_"<< 
				tmp_string_user << "_" << tmp_string_time << ".mrd";

	return file_name_ss.str();
}


/*****************************************************************************/

char *last_data()
{
	FILE *stream;
	int file_number;

	stream = fopen( "last_data", "r" );
	if ( stream == NULL )
		return strdup( "d00000" );

	if(fscanf( stream, "%d", &file_number ) == 0)
		return nullptr;

	sprintf( generated_file_name, "d%05d", file_number - 1 );
	fclose( stream );
	return strdup( generated_file_name );
}

/*****************************************************************************/
/*****************************************************************************/


