/**
 * File: popfilebuffer.h
 * Author: Valentin Clement (clementval)
 * Description: Implementation of the POPFileBuffer. This Object holds data before sending them to the POPFileManager. 
 * Creation date: 04.23.2012
 * 
 * Change Log: 
 * Author		Date			Description
 * clementval	04.23.2012	Creation of this file
 */
#include "popfilebuffer.h"
#include "popfilemanager.ph"
#include "paroc_thread.h"
#include <sys/time.h>
#include <unistd.h>
//#include <semaphor.h>


using namespace popfile;


const char* POPFileBuffer::POPFILEBUFFER_FULL_WITHOUT_REMAINING = "FULL";

/**
 * POPFileBuffer Constructor
 */
POPFileBuffer::POPFileBuffer(){
	capacity = 1024;
	remainingCapacity = capacity;
	identifier = 0;
}

/**
 * POPFileBuffer Destructor
 */
POPFileBuffer::~POPFileBuffer(){
	delete pfmref;
}

/**
 * add string value to the buffer and manage flushing mechanism
 * @param value 
 * @return remaining
 */
std::string POPFileBuffer::buffer_add(std::string value){
	std::string remaining;
	if(value.length() < remainingCapacity){
		popfile_buffer_out << value;	
		remainingCapacity -= value.length();
	} else {
		remaining = value.substr(remainingCapacity);
		std::string toadd = value.substr(0, remainingCapacity);
		remainingCapacity -= toadd.length();
		popfile_buffer_out << toadd;	
		flush();
		if(remaining.compare("") == 0)
			remaining = POPFILEBUFFER_FULL_WITHOUT_REMAINING;
	}
	return remaining;
}

/**
 * Flush the buffer to the actual strip when the capacity is exceed
 * @return void 
 */
void POPFileBuffer::flush(){
	if(remainingCapacity < capacity){
		struct timeval start1, end1, start2, end2, start3, end3;
	   long mtime, seconds, useconds; 

		gettimeofday(&start1, NULL);	
		POPString data(popfile_buffer_out.str().c_str());
		localpfmref->writeToRemoteStrip(stripPath, data, associatedPFM);
		gettimeofday(&end1, NULL);		
		seconds  = end1.tv_sec  - start1.tv_sec;
   	useconds = end1.tv_usec - start1.tv_usec;
	   mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
		
		cout << "[POPFILEBUFFER] Flushing buffer[" << identifier << "] " << " (Remaining Capacity="<< remainingCapacity  <<") - (Data flushed=" << popfile_buffer_out.str().length() << "): time [ms]" << mtime << popcendl;
		
		
		popfile_buffer_out.str("");
		remainingCapacity = capacity;		
	}
}

/**
 * Read from internal buffer
 * @param	size to be read in the buffer
 * @return 	Data read from the buffer as a string 
 */
std::string POPFileBuffer::buffer_get(long size)
{
	if(popfile_buffer_in.str().length() < size){
		
		cout << "[POPFILEBUFFER] Not enough data in buffer" << popcendl;
	} else {
		std::string data = popfile_buffer_in.str().substr(0, size);
		popfile_buffer_in.str(popfile_buffer_in.str().substr(size));	
		return data;
	}
}

/**
 * Get a block data of the offset size from the buffer.
 * @return A string object representing the data block read
 */
std::string POPFileBuffer::buffer_get_offset()
{
	return buffer_get(capacity);
}

/**
 * Get the size of data currently in the input buffer.
 * @return The size of data in the buffer
 */
long POPFileBuffer::get_size_input_data()
{
	return popfile_buffer_in.str().length();
}

/**
 * Add data into the buffer 
 * @param data The data to add in the buffer
 * @return void
 */
void POPFileBuffer::add_data(POPString data)
{
	popfile_buffer_in << data.GetString();
}

/**
 * Set the capacity of the buffer.
 * @param value	The capacity to set for this buffer
 * @return void 
 */
void POPFileBuffer::set_capacity(long value)
{
	capacity = value;	
	remainingCapacity = capacity;	
}

/**
 * Set the identifier of this buffer (associated strip)
 * @param value The identifier to set for this buffer
 * @return void 
 */
void POPFileBuffer::set_identifier(int value)
{
	identifier = value;
}

/**
 * Set the associated strip path
 * @param value The associated strip path to be set with this buffer
 * @return void 
 */
void POPFileBuffer::set_strip_path(std::string value)
{
	stripPath = value.c_str();
}

/**
 * Set the associated PFM accesspoint
 * @param ap The accesspoint of the associated PFM
 * @return void 
 */
void POPFileBuffer::setAssociatedPOPFileManager(paroc_accesspoint ap)
{
	associatedPFM = ap;
	pfmref = new POPFileManager(associatedPFM);
}

/**
 * Get the associated PFM accesspoint
 * @return The accesspoint of the associated PFM
 * @return void 
 */
paroc_accesspoint POPFileBuffer::get_associated_pfm()
{
	return associatedPFM;
}

/**
 * Set the local PFM accesspoint
 * @param ap The accesspoint of the local PFM
 * @return void 
 */
void POPFileBuffer::setLocalPOPFileManager(paroc_accesspoint ap)
{
	localpfmref = new POPFileManager(ap);
}