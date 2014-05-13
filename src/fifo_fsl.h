/*
 * Timed FIFO
 * Sven van Haastregt
 * LERC, LIACS, Leiden University
 * Version: $Id: fifo_fsl.h,v 1.3 2011/04/15 11:50:14 svhaastr Exp $
 */
#ifndef _FIFO_FSL_H_
#define _FIFO_FSL_H_

#include "systemc.h"

// Usage notes:
// Each cycle only one read and one write operation are allowed. If data is read/written (that is, the operation doesn't block
// on an empty or full FIFO), time is not advanced: advancing time should be done by the caller.

// Set to nonzero to print FIFO operations
//#define FIFO_VERBOSE 1

// TODO: move to constructor of fifo? the user may want to set this from the command line
const bool fail_on_blocking = true;

template <class T>
SC_MODULE(fsl) , public sc_fifo<T> {
  public:
    sc_in<bool> clk;
//     sc_in<bool> rst;

    fsl(sc_module_name mn, int size);
    fsl(sc_module_name mn, int size, sc_trace_file *tf);
    //fsl(sc_module_name mn, int size, int rlatency, int wlatency);
    ~fsl();

    sc_signal<bool> exist;
    sc_signal<bool> full;

    void read( T& );
    void write(const T&);

    SC_HAS_PROCESS(fsl);

  private:
    void init(int size, sc_trace_file *tf);
    void fsl_process();

    sc_trace_file *tf;
    int read_pending;
    T read_val;
    sc_event read_done;
    bool *read_pipeline;
    bool *write_pipeline;
    T *read_queue;
    T *write_queue;
    int read_latency;     // Time between read call and return (simulates time to fetch data from buffer)
    int write_latency;    // Time between write call and actual write into buffer; write() returns immediately if not blocking! Multiple writes in subsequent cycles are pipelined.
    int flen;             // Resembling fsl.fifo_length
    int max_tokens;       // Maximum number of tokens simultaneously in FIFO (collected during execution)
};


template <class T>
fsl<T>::fsl(sc_module_name mn, int size) : sc_fifo<T> (size) {
  init(size, NULL);
}


template <class T>
fsl<T>::fsl(sc_module_name mn, int size, sc_trace_file *tf) : sc_fifo<T> (size) {
  init(size, tf);
}


template <class T>
fsl<T>::~fsl() {
  delete[] read_pipeline;
  delete[] read_queue;
  delete[] write_pipeline;
  delete[] write_queue;
  cout << sc_module::name() << ": Maximum number of tokens simultaneously in FIFO: " << max_tokens << endl;
}


template <class T>
void fsl<T>::init(int size, sc_trace_file *tf) {
  read_pending = -1;
  read_latency = 1;   // Values != 1 not tested
  write_latency = 1;  // Values != 1 not tested
  SC_CTHREAD(fsl_process, clk.pos());
  this->read_pipeline = new bool[read_latency+1];
  this->read_queue = new T[read_latency+1];
  this->write_pipeline = new bool[write_latency+1];
  this->write_queue = new T[write_latency+1];

  for (int i = 0; i <= read_latency; i++) {
    read_pipeline[i] = false;
  }
  for (int i = 0; i <= write_latency; i++) {
    write_pipeline[i] = false;
  }

  this->tf = tf;
  if (tf) {
    std::string sige("E");
    sige.append(sc_module::name());
    sige.append(".exist");
    sc_trace(tf, exist, sige);
    std::string sigf("E");
    sigf.append(sc_module::name());
    sigf.append(".full");
    sc_trace(tf, full, sigf);
  }

  flen = 0;
  exist.write(false);
  full.write(false);
  max_tokens = 0;
}


template <class T>
void fsl<T>::read( T& val_ )
{
  if (this->read_pending == 1) {
    cerr << "[" << sc_module::name() << "] Error: more than one read during cycle " << sc_time_stamp() << endl;
    exit(1);
  }

  // Block if empty
  while( this->num_available() == 0 ) {
    if (fail_on_blocking) {
      cerr << sc_module::name() << ": Read at " << sc_time_stamp() << " blocked, aborting" << endl;
      exit(1);
    }
#ifdef FIFO_VERBOSE
    cout << sc_module::name() << ": Blocking read at " << sc_time_stamp() << endl;
#endif
    sc_core::wait(1, SC_NS);
  }

  // Do actual read in next clock cycle
  this->read_pending = 1;//read_latency;

  this->m_num_read++;
  this->buf_read(this->read_val);
  this->request_update();

  val_ = read_val;
}

template <class T>
void fsl<T>::write(const T& val) {
#ifdef FIFO_VERBOSE
  cout << "Write " << sc_module::name() << " at " << sc_time_stamp() << endl;
#endif
  if (write_pipeline[0] == true) {
    cerr << "[" << sc_module::name() << "] Error: more than one write during cycle " << sc_time_stamp() << endl;
    exit(1);
  }

  // Block if full
  while (this->num_free() == 0) {
    if (fail_on_blocking) {
      cerr << sc_module::name() << ": Write at " << sc_time_stamp() << " blocked, aborting" << endl;
      exit(1);
    }
#ifdef FIFO_VERBOSE
    cout << sc_module::name() << ": Blocking write at " << sc_time_stamp() << endl;
#endif
    // TODO: shouldn't this be a wait 1 SC_NS as well..?
    sc_core::wait( this->m_data_read_event );
    //sc_core::wait(1, SC_NS);
  }

  // Queue the write operation
  write_queue[0] = val;
  write_pipeline[0] = true;
}


// Clocked process
template <class T>
void fsl<T>::fsl_process() {
  while (1) {
    // Advance write queue:
    for (int i = write_latency; i > 0; i--) {
      write_pipeline[i] = write_pipeline[i-1];
      write_queue[i] = write_queue[i-1];
    }
    write_pipeline[0] = false;

    if (this->read_pending != 1  &&  write_pipeline[write_latency] == true  &&  full.read()==false) {
      // write and no read: increment
      flen++;
    }
    else if (this->read_pending == 1  &&  write_pipeline[write_latency] == false  &&  exist.read()==true) {
      // read and no write: decrement
      flen--;
    }
    exist.write(flen != 0);
    full.write(flen == this->m_size);

    // Keep track of maximum number of tokens simultaneously in FIFO (which is the maximum buffersize for self-timed execution)
    if (flen > max_tokens)
      max_tokens = flen;

    // Handle read
    if (this->read_pending > 0) {
      this->read_pending--;
    }

    if (write_pipeline[write_latency] == true) {
      // A write is coming out of the queue, commit it:
#ifdef FIFO_VERBOSE
      cout << sc_module::name() << " committing write at " << sc_time_stamp() << endl;
#endif
      this->m_num_written++;
      this->buf_write(write_queue[write_latency]);
      this->request_update();
    }

    sc_core::wait();
  }
}


#endif
