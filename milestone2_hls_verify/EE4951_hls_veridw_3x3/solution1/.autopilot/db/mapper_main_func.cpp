#include "hls_signal_handler.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <vector>
#include "ap_fixed.h"
#include "ap_int.h"
#include "hls_stream.h"
using namespace std;

namespace hls::sim
{
  template<size_t n>
  struct Byte {
    unsigned char a[n];

    Byte()
    {
      for (size_t i = 0; i < n; ++i) {
        a[i] = 0;
      }
    }

    template<typename T>
    Byte<n>& operator= (const T &val)
    {
      std::memcpy(a, &val, n);
      return *this;
    }
  };

  struct SimException : public std::exception {
    const std::string msg;
    const size_t line;
    SimException(const std::string &msg, const size_t line)
      : msg(msg), line(line)
    {
    }
  };

  void errExit(const size_t line, const std::string &msg)
  {
    std::string s;
    s += "ERROR";
//  s += '(';
//  s += __FILE__;
//  s += ":";
//  s += std::to_string(line);
//  s += ')';
    s += ": ";
    s += msg;
    s += "\n";
    fputs(s.c_str(), stderr);
    exit(1);
  }
}


namespace hls::sim
{
  struct Buffer {
    char *first;
    Buffer(char *addr) : first(addr)
    {
    }
  };

  struct DBuffer : public Buffer {
    static const size_t total = 1<<10;
    size_t ufree;

    DBuffer(size_t usize) : Buffer(nullptr), ufree(total)
    {
      first = new char[usize*ufree];
    }

    ~DBuffer()
    {
      delete[] first;
    }
  };

  struct CStream {
    char *front;
    char *back;
    size_t num;
    size_t usize;
    std::list<Buffer*> bufs;
    bool dynamic;

    CStream() : front(nullptr), back(nullptr),
                num(0), usize(0), dynamic(true)
    {
    }

    ~CStream()
    {
      for (Buffer *p : bufs) {
        delete p;
      }
    }

    template<typename T>
    T* data()
    {
      return (T*)front;
    }

    template<typename T>
    void transfer(hls::stream<T> *param)
    {
      while (!empty()) {
        param->write(*(T*)nextRead());
      }
    }

    bool empty();
    char* nextRead();
    char* nextWrite();
  };

  bool CStream::empty()
  {
    return num == 0;
  }

  char* CStream::nextRead()
  {
    assert(num > 0);
    char *res = front;
    front += usize;
    if (dynamic) {
      if (++static_cast<DBuffer*>(bufs.front())->ufree == DBuffer::total) {
        if (bufs.size() > 1) {
          bufs.pop_front();
          front = bufs.front()->first;
        } else {
          front = back = bufs.front()->first;
        }
      }
    }
    --num;
    return res;
  }

  char* CStream::nextWrite()
  {
    if (dynamic) {
      if (static_cast<DBuffer*>(bufs.back())->ufree == 0) {
        bufs.push_back(new DBuffer(usize));
        back = bufs.back()->first;
      }
      --static_cast<DBuffer*>(bufs.back())->ufree;
    }
    char *res = back;
    back += usize;
    ++num;
    return res;
  }

  std::list<CStream> streams;
  std::map<char*, CStream*> prebuilt;

  CStream* createStream(size_t usize)
  {
    streams.emplace_front();
    CStream &s = streams.front();
    {
      s.dynamic = true;
      s.bufs.push_back(new DBuffer(usize));
      s.front = s.bufs.back()->first;
      s.back = s.front;
      s.num = 0;
      s.usize = usize;
    }
    return &s;
  }

  template<typename T>
  CStream* createStream(hls::stream<T> *param)
  {
    CStream *s = createStream(sizeof(T));
    {
      s->dynamic = true;
      while (!param->empty()) {
        T data = param->read();
        memcpy(s->nextWrite(), (char*)&data, sizeof(T));
      }
      prebuilt[s->front] = s;
    }
    return s;
  }

  template<typename T>
  CStream* createStream(T *param, size_t usize)
  {
    streams.emplace_front();
    CStream &s = streams.front();
    {
      s.dynamic = false;
      s.bufs.push_back(new Buffer((char*)param));
      s.front = s.back = s.bufs.back()->first;
      s.usize = usize;
      s.num = ~0UL;
    }
    prebuilt[s.front] = &s;
    return &s;
  }

  CStream* findStream(char *buf)
  {
    return prebuilt.at(buf);
  }
}
class AESL_RUNTIME_BC {
  public:
    AESL_RUNTIME_BC(const char* name) {
      file_token.open( name);
      if (!file_token.good()) {
        cout << "Failed to open tv file " << name << endl;
        exit (1);
      }
      file_token >> mName;//[[[runtime]]]
    }
    ~AESL_RUNTIME_BC() {
      file_token.close();
    }
    int read_size () {
      int size = 0;
      file_token >> mName;//[[transaction]]
      file_token >> mName;//transaction number
      file_token >> mName;//pop_size
      size = atoi(mName.c_str());
      file_token >> mName;//[[/transaction]]
      return size;
    }
  public:
    fstream file_token;
    string mName;
};
using hls::sim::Byte;
extern "C" void main_func(Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, Byte<2>*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
extern "C" void apatb_main_func_hw(volatile void * __xlx_apatb_param_input_feature_map, volatile void * __xlx_apatb_param_layer_weights, volatile void * __xlx_apatb_param_layer_bias, volatile void * __xlx_apatb_param_output_feature_map_0, volatile void * __xlx_apatb_param_output_feature_map_1, volatile void * __xlx_apatb_param_output_feature_map_2, volatile void * __xlx_apatb_param_output_feature_map_3, volatile void * __xlx_apatb_param_output_feature_map_4, volatile void * __xlx_apatb_param_output_feature_map_5, volatile void * __xlx_apatb_param_output_feature_map_6, volatile void * __xlx_apatb_param_output_feature_map_7, volatile void * __xlx_apatb_param_output_feature_map_8, volatile void * __xlx_apatb_param_output_feature_map_9, volatile void * __xlx_apatb_param_output_feature_map_10, volatile void * __xlx_apatb_param_output_feature_map_11, volatile void * __xlx_apatb_param_output_feature_map_12, volatile void * __xlx_apatb_param_output_feature_map_13, volatile void * __xlx_apatb_param_output_feature_map_14, volatile void * __xlx_apatb_param_output_feature_map_15, volatile void * __xlx_apatb_param_output_feature_map_16, volatile void * __xlx_apatb_param_output_feature_map_17, volatile void * __xlx_apatb_param_output_feature_map_18, volatile void * __xlx_apatb_param_output_feature_map_19, volatile void * __xlx_apatb_param_output_feature_map_20, volatile void * __xlx_apatb_param_output_feature_map_21, volatile void * __xlx_apatb_param_output_feature_map_22, volatile void * __xlx_apatb_param_output_feature_map_23, volatile void * __xlx_apatb_param_output_feature_map_24, volatile void * __xlx_apatb_param_output_feature_map_25, volatile void * __xlx_apatb_param_output_feature_map_26, volatile void * __xlx_apatb_param_output_feature_map_27, volatile void * __xlx_apatb_param_output_feature_map_28, volatile void * __xlx_apatb_param_output_feature_map_29, volatile void * __xlx_apatb_param_output_feature_map_30, volatile void * __xlx_apatb_param_output_feature_map_31, volatile void * __xlx_apatb_param_output_feature_map_32, volatile void * __xlx_apatb_param_output_feature_map_33, volatile void * __xlx_apatb_param_output_feature_map_34, volatile void * __xlx_apatb_param_output_feature_map_35, volatile void * __xlx_apatb_param_output_feature_map_36, volatile void * __xlx_apatb_param_output_feature_map_37, volatile void * __xlx_apatb_param_output_feature_map_38, volatile void * __xlx_apatb_param_output_feature_map_39, volatile void * __xlx_apatb_param_output_feature_map_40, volatile void * __xlx_apatb_param_output_feature_map_41, volatile void * __xlx_apatb_param_output_feature_map_42, volatile void * __xlx_apatb_param_output_feature_map_43, volatile void * __xlx_apatb_param_output_feature_map_44, volatile void * __xlx_apatb_param_output_feature_map_45, volatile void * __xlx_apatb_param_output_feature_map_46, volatile void * __xlx_apatb_param_output_feature_map_47, volatile void * __xlx_apatb_param_output_feature_map_48, volatile void * __xlx_apatb_param_output_feature_map_49, volatile void * __xlx_apatb_param_output_feature_map_50, volatile void * __xlx_apatb_param_output_feature_map_51, volatile void * __xlx_apatb_param_output_feature_map_52, volatile void * __xlx_apatb_param_output_feature_map_53, volatile void * __xlx_apatb_param_output_feature_map_54, volatile void * __xlx_apatb_param_output_feature_map_55, volatile void * __xlx_apatb_param_output_feature_map_56, volatile void * __xlx_apatb_param_output_feature_map_57, volatile void * __xlx_apatb_param_output_feature_map_58, volatile void * __xlx_apatb_param_output_feature_map_59, volatile void * __xlx_apatb_param_output_feature_map_60, volatile void * __xlx_apatb_param_output_feature_map_61, volatile void * __xlx_apatb_param_output_feature_map_62, volatile void * __xlx_apatb_param_output_feature_map_63, volatile void * __xlx_apatb_param_output_feature_map_64, volatile void * __xlx_apatb_param_output_feature_map_65, volatile void * __xlx_apatb_param_output_feature_map_66, volatile void * __xlx_apatb_param_output_feature_map_67, volatile void * __xlx_apatb_param_output_feature_map_68, volatile void * __xlx_apatb_param_output_feature_map_69, volatile void * __xlx_apatb_param_output_feature_map_70, volatile void * __xlx_apatb_param_output_feature_map_71, volatile void * __xlx_apatb_param_output_feature_map_72, volatile void * __xlx_apatb_param_output_feature_map_73, volatile void * __xlx_apatb_param_output_feature_map_74, volatile void * __xlx_apatb_param_output_feature_map_75, volatile void * __xlx_apatb_param_output_feature_map_76, volatile void * __xlx_apatb_param_output_feature_map_77, volatile void * __xlx_apatb_param_output_feature_map_78, volatile void * __xlx_apatb_param_output_feature_map_79, volatile void * __xlx_apatb_param_output_feature_map_80, volatile void * __xlx_apatb_param_output_feature_map_81, volatile void * __xlx_apatb_param_output_feature_map_82, volatile void * __xlx_apatb_param_output_feature_map_83, volatile void * __xlx_apatb_param_output_feature_map_84, volatile void * __xlx_apatb_param_output_feature_map_85, volatile void * __xlx_apatb_param_output_feature_map_86, volatile void * __xlx_apatb_param_output_feature_map_87, volatile void * __xlx_apatb_param_output_feature_map_88, volatile void * __xlx_apatb_param_output_feature_map_89, volatile void * __xlx_apatb_param_output_feature_map_90, volatile void * __xlx_apatb_param_output_feature_map_91, volatile void * __xlx_apatb_param_output_feature_map_92, volatile void * __xlx_apatb_param_output_feature_map_93, volatile void * __xlx_apatb_param_output_feature_map_94, volatile void * __xlx_apatb_param_output_feature_map_95, volatile void * __xlx_apatb_param_output_feature_map_96, volatile void * __xlx_apatb_param_output_feature_map_97, volatile void * __xlx_apatb_param_output_feature_map_98, volatile void * __xlx_apatb_param_output_feature_map_99, volatile void * __xlx_apatb_param_output_feature_map_100, volatile void * __xlx_apatb_param_output_feature_map_101, volatile void * __xlx_apatb_param_output_feature_map_102, volatile void * __xlx_apatb_param_output_feature_map_103, volatile void * __xlx_apatb_param_output_feature_map_104, volatile void * __xlx_apatb_param_output_feature_map_105, volatile void * __xlx_apatb_param_output_feature_map_106, volatile void * __xlx_apatb_param_output_feature_map_107, volatile void * __xlx_apatb_param_output_feature_map_108, volatile void * __xlx_apatb_param_output_feature_map_109, volatile void * __xlx_apatb_param_output_feature_map_110, volatile void * __xlx_apatb_param_output_feature_map_111, volatile void * __xlx_apatb_param_output_feature_map_112, volatile void * __xlx_apatb_param_output_feature_map_113, volatile void * __xlx_apatb_param_output_feature_map_114, volatile void * __xlx_apatb_param_output_feature_map_115, volatile void * __xlx_apatb_param_output_feature_map_116, volatile void * __xlx_apatb_param_output_feature_map_117, volatile void * __xlx_apatb_param_output_feature_map_118, volatile void * __xlx_apatb_param_output_feature_map_119, volatile void * __xlx_apatb_param_output_feature_map_120, volatile void * __xlx_apatb_param_output_feature_map_121, volatile void * __xlx_apatb_param_output_feature_map_122, volatile void * __xlx_apatb_param_output_feature_map_123, volatile void * __xlx_apatb_param_output_feature_map_124, volatile void * __xlx_apatb_param_output_feature_map_125, volatile void * __xlx_apatb_param_output_feature_map_126, volatile void * __xlx_apatb_param_output_feature_map_127, volatile void * __xlx_apatb_param_output_feature_map_128, volatile void * __xlx_apatb_param_output_feature_map_129, volatile void * __xlx_apatb_param_output_feature_map_130, volatile void * __xlx_apatb_param_output_feature_map_131, volatile void * __xlx_apatb_param_output_feature_map_132, volatile void * __xlx_apatb_param_output_feature_map_133, volatile void * __xlx_apatb_param_output_feature_map_134, volatile void * __xlx_apatb_param_output_feature_map_135, volatile void * __xlx_apatb_param_output_feature_map_136, volatile void * __xlx_apatb_param_output_feature_map_137, volatile void * __xlx_apatb_param_output_feature_map_138, volatile void * __xlx_apatb_param_output_feature_map_139, volatile void * __xlx_apatb_param_output_feature_map_140, volatile void * __xlx_apatb_param_output_feature_map_141, volatile void * __xlx_apatb_param_output_feature_map_142, volatile void * __xlx_apatb_param_output_feature_map_143, volatile void * __xlx_apatb_param_output_feature_map_144, volatile void * __xlx_apatb_param_output_feature_map_145, volatile void * __xlx_apatb_param_output_feature_map_146, volatile void * __xlx_apatb_param_output_feature_map_147, volatile void * __xlx_apatb_param_output_feature_map_148, volatile void * __xlx_apatb_param_output_feature_map_149, volatile void * __xlx_apatb_param_output_feature_map_150, volatile void * __xlx_apatb_param_output_feature_map_151, volatile void * __xlx_apatb_param_output_feature_map_152, volatile void * __xlx_apatb_param_output_feature_map_153, volatile void * __xlx_apatb_param_output_feature_map_154, volatile void * __xlx_apatb_param_output_feature_map_155, volatile void * __xlx_apatb_param_output_feature_map_156, volatile void * __xlx_apatb_param_output_feature_map_157, volatile void * __xlx_apatb_param_output_feature_map_158, volatile void * __xlx_apatb_param_output_feature_map_159, volatile void * __xlx_apatb_param_output_feature_map_160, volatile void * __xlx_apatb_param_output_feature_map_161, volatile void * __xlx_apatb_param_output_feature_map_162, volatile void * __xlx_apatb_param_output_feature_map_163, volatile void * __xlx_apatb_param_output_feature_map_164, volatile void * __xlx_apatb_param_output_feature_map_165, volatile void * __xlx_apatb_param_output_feature_map_166, volatile void * __xlx_apatb_param_output_feature_map_167, volatile void * __xlx_apatb_param_output_feature_map_168, volatile void * __xlx_apatb_param_output_feature_map_169, volatile void * __xlx_apatb_param_output_feature_map_170, volatile void * __xlx_apatb_param_output_feature_map_171, volatile void * __xlx_apatb_param_output_feature_map_172, volatile void * __xlx_apatb_param_output_feature_map_173, volatile void * __xlx_apatb_param_output_feature_map_174, volatile void * __xlx_apatb_param_output_feature_map_175, volatile void * __xlx_apatb_param_output_feature_map_176, volatile void * __xlx_apatb_param_output_feature_map_177, volatile void * __xlx_apatb_param_output_feature_map_178, volatile void * __xlx_apatb_param_output_feature_map_179, volatile void * __xlx_apatb_param_output_feature_map_180, volatile void * __xlx_apatb_param_output_feature_map_181, volatile void * __xlx_apatb_param_output_feature_map_182, volatile void * __xlx_apatb_param_output_feature_map_183, volatile void * __xlx_apatb_param_output_feature_map_184, volatile void * __xlx_apatb_param_output_feature_map_185, volatile void * __xlx_apatb_param_output_feature_map_186, volatile void * __xlx_apatb_param_output_feature_map_187, volatile void * __xlx_apatb_param_output_feature_map_188, volatile void * __xlx_apatb_param_output_feature_map_189, volatile void * __xlx_apatb_param_output_feature_map_190, volatile void * __xlx_apatb_param_output_feature_map_191, volatile void * __xlx_apatb_param_output_feature_map_192, volatile void * __xlx_apatb_param_output_feature_map_193, volatile void * __xlx_apatb_param_output_feature_map_194, volatile void * __xlx_apatb_param_output_feature_map_195, volatile void * __xlx_apatb_param_output_feature_map_196, volatile void * __xlx_apatb_param_output_feature_map_197, volatile void * __xlx_apatb_param_output_feature_map_198, volatile void * __xlx_apatb_param_output_feature_map_199, volatile void * __xlx_apatb_param_output_feature_map_200, volatile void * __xlx_apatb_param_output_feature_map_201, volatile void * __xlx_apatb_param_output_feature_map_202, volatile void * __xlx_apatb_param_output_feature_map_203, volatile void * __xlx_apatb_param_output_feature_map_204, volatile void * __xlx_apatb_param_output_feature_map_205, volatile void * __xlx_apatb_param_output_feature_map_206, volatile void * __xlx_apatb_param_output_feature_map_207, volatile void * __xlx_apatb_param_output_feature_map_208, volatile void * __xlx_apatb_param_output_feature_map_209, volatile void * __xlx_apatb_param_output_feature_map_210, volatile void * __xlx_apatb_param_output_feature_map_211, volatile void * __xlx_apatb_param_output_feature_map_212, volatile void * __xlx_apatb_param_output_feature_map_213, volatile void * __xlx_apatb_param_output_feature_map_214, volatile void * __xlx_apatb_param_output_feature_map_215, volatile void * __xlx_apatb_param_output_feature_map_216, volatile void * __xlx_apatb_param_output_feature_map_217, volatile void * __xlx_apatb_param_output_feature_map_218, volatile void * __xlx_apatb_param_output_feature_map_219, volatile void * __xlx_apatb_param_output_feature_map_220, volatile void * __xlx_apatb_param_output_feature_map_221, volatile void * __xlx_apatb_param_output_feature_map_222, volatile void * __xlx_apatb_param_output_feature_map_223, volatile void * __xlx_apatb_param_output_feature_map_224, volatile void * __xlx_apatb_param_output_feature_map_225, volatile void * __xlx_apatb_param_output_feature_map_226, volatile void * __xlx_apatb_param_output_feature_map_227, volatile void * __xlx_apatb_param_output_feature_map_228, volatile void * __xlx_apatb_param_output_feature_map_229, volatile void * __xlx_apatb_param_output_feature_map_230, volatile void * __xlx_apatb_param_output_feature_map_231, volatile void * __xlx_apatb_param_output_feature_map_232, volatile void * __xlx_apatb_param_output_feature_map_233, volatile void * __xlx_apatb_param_output_feature_map_234, volatile void * __xlx_apatb_param_output_feature_map_235, volatile void * __xlx_apatb_param_output_feature_map_236, volatile void * __xlx_apatb_param_output_feature_map_237, volatile void * __xlx_apatb_param_output_feature_map_238, volatile void * __xlx_apatb_param_output_feature_map_239, volatile void * __xlx_apatb_param_output_feature_map_240, volatile void * __xlx_apatb_param_output_feature_map_241, volatile void * __xlx_apatb_param_output_feature_map_242, volatile void * __xlx_apatb_param_output_feature_map_243, volatile void * __xlx_apatb_param_output_feature_map_244, volatile void * __xlx_apatb_param_output_feature_map_245, volatile void * __xlx_apatb_param_output_feature_map_246, volatile void * __xlx_apatb_param_output_feature_map_247, volatile void * __xlx_apatb_param_output_feature_map_248, volatile void * __xlx_apatb_param_output_feature_map_249, volatile void * __xlx_apatb_param_output_feature_map_250, volatile void * __xlx_apatb_param_output_feature_map_251, volatile void * __xlx_apatb_param_output_feature_map_252, volatile void * __xlx_apatb_param_output_feature_map_253, volatile void * __xlx_apatb_param_output_feature_map_254, volatile void * __xlx_apatb_param_output_feature_map_255, volatile void * __xlx_apatb_param_output_feature_map_256, volatile void * __xlx_apatb_param_output_feature_map_257, volatile void * __xlx_apatb_param_output_feature_map_258, volatile void * __xlx_apatb_param_output_feature_map_259, volatile void * __xlx_apatb_param_output_feature_map_260, volatile void * __xlx_apatb_param_output_feature_map_261, volatile void * __xlx_apatb_param_output_feature_map_262, volatile void * __xlx_apatb_param_output_feature_map_263, volatile void * __xlx_apatb_param_output_feature_map_264, volatile void * __xlx_apatb_param_output_feature_map_265, volatile void * __xlx_apatb_param_output_feature_map_266, volatile void * __xlx_apatb_param_output_feature_map_267, volatile void * __xlx_apatb_param_output_feature_map_268, volatile void * __xlx_apatb_param_output_feature_map_269, volatile void * __xlx_apatb_param_output_feature_map_270, volatile void * __xlx_apatb_param_output_feature_map_271, volatile void * __xlx_apatb_param_output_feature_map_272, volatile void * __xlx_apatb_param_output_feature_map_273, volatile void * __xlx_apatb_param_output_feature_map_274, volatile void * __xlx_apatb_param_output_feature_map_275, volatile void * __xlx_apatb_param_output_feature_map_276, volatile void * __xlx_apatb_param_output_feature_map_277, volatile void * __xlx_apatb_param_output_feature_map_278, volatile void * __xlx_apatb_param_output_feature_map_279, volatile void * __xlx_apatb_param_output_feature_map_280, volatile void * __xlx_apatb_param_output_feature_map_281, volatile void * __xlx_apatb_param_output_feature_map_282, volatile void * __xlx_apatb_param_output_feature_map_283, volatile void * __xlx_apatb_param_output_feature_map_284, volatile void * __xlx_apatb_param_output_feature_map_285, volatile void * __xlx_apatb_param_output_feature_map_286, volatile void * __xlx_apatb_param_output_feature_map_287, volatile void * __xlx_apatb_param_output_feature_map_288, volatile void * __xlx_apatb_param_output_feature_map_289, volatile void * __xlx_apatb_param_output_feature_map_290, volatile void * __xlx_apatb_param_output_feature_map_291, volatile void * __xlx_apatb_param_output_feature_map_292, volatile void * __xlx_apatb_param_output_feature_map_293, volatile void * __xlx_apatb_param_output_feature_map_294, volatile void * __xlx_apatb_param_output_feature_map_295, volatile void * __xlx_apatb_param_output_feature_map_296, volatile void * __xlx_apatb_param_output_feature_map_297, volatile void * __xlx_apatb_param_output_feature_map_298, volatile void * __xlx_apatb_param_output_feature_map_299, volatile void * __xlx_apatb_param_output_feature_map_300, volatile void * __xlx_apatb_param_output_feature_map_301, volatile void * __xlx_apatb_param_output_feature_map_302, volatile void * __xlx_apatb_param_output_feature_map_303, volatile void * __xlx_apatb_param_output_feature_map_304, volatile void * __xlx_apatb_param_output_feature_map_305, volatile void * __xlx_apatb_param_output_feature_map_306, volatile void * __xlx_apatb_param_output_feature_map_307, volatile void * __xlx_apatb_param_output_feature_map_308, volatile void * __xlx_apatb_param_output_feature_map_309, volatile void * __xlx_apatb_param_output_feature_map_310, volatile void * __xlx_apatb_param_output_feature_map_311, volatile void * __xlx_apatb_param_output_feature_map_312, volatile void * __xlx_apatb_param_output_feature_map_313, volatile void * __xlx_apatb_param_output_feature_map_314, volatile void * __xlx_apatb_param_output_feature_map_315, volatile void * __xlx_apatb_param_output_feature_map_316, volatile void * __xlx_apatb_param_output_feature_map_317, volatile void * __xlx_apatb_param_output_feature_map_318, volatile void * __xlx_apatb_param_output_feature_map_319) {
using hls::sim::createStream;
  // Collect __xlx_input_feature_map__tmp_vec
std::vector<Byte<2>> __xlx_input_feature_map__tmp_vec;
for (size_t i = 0; i < 153600; ++i){
__xlx_input_feature_map__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_input_feature_map)[i]);
}
  int __xlx_size_param_input_feature_map = 153600;
  int __xlx_offset_param_input_feature_map = 0;
  int __xlx_offset_byte_param_input_feature_map = 0*2;
  // Collect __xlx_layer_weights_layer_bias__tmp_vec
std::vector<Byte<2>> __xlx_layer_weights_layer_bias__tmp_vec;
for (size_t i = 0; i < 81; ++i){
__xlx_layer_weights_layer_bias__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_layer_weights)[i]);
}
  int __xlx_size_param_layer_weights = 81;
  int __xlx_offset_param_layer_weights = 0;
  int __xlx_offset_byte_param_layer_weights = 0*2;
for (size_t i = 0; i < 3; ++i){
__xlx_layer_weights_layer_bias__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_layer_bias)[i]);
}
  int __xlx_size_param_layer_bias = 3;
  int __xlx_offset_param_layer_bias = 81;
  int __xlx_offset_byte_param_layer_bias = 81*2;
  // Collect __xlx_output_feature_map_0__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_0__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_0__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_0)[i]);
}
  int __xlx_size_param_output_feature_map_0 = 480;
  int __xlx_offset_param_output_feature_map_0 = 0;
  int __xlx_offset_byte_param_output_feature_map_0 = 0*2;
  // Collect __xlx_output_feature_map_1__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_1__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_1__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_1)[i]);
}
  int __xlx_size_param_output_feature_map_1 = 480;
  int __xlx_offset_param_output_feature_map_1 = 0;
  int __xlx_offset_byte_param_output_feature_map_1 = 0*2;
  // Collect __xlx_output_feature_map_2__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_2__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_2__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_2)[i]);
}
  int __xlx_size_param_output_feature_map_2 = 480;
  int __xlx_offset_param_output_feature_map_2 = 0;
  int __xlx_offset_byte_param_output_feature_map_2 = 0*2;
  // Collect __xlx_output_feature_map_3__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_3__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_3__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_3)[i]);
}
  int __xlx_size_param_output_feature_map_3 = 480;
  int __xlx_offset_param_output_feature_map_3 = 0;
  int __xlx_offset_byte_param_output_feature_map_3 = 0*2;
  // Collect __xlx_output_feature_map_4__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_4__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_4__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_4)[i]);
}
  int __xlx_size_param_output_feature_map_4 = 480;
  int __xlx_offset_param_output_feature_map_4 = 0;
  int __xlx_offset_byte_param_output_feature_map_4 = 0*2;
  // Collect __xlx_output_feature_map_5__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_5__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_5__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_5)[i]);
}
  int __xlx_size_param_output_feature_map_5 = 480;
  int __xlx_offset_param_output_feature_map_5 = 0;
  int __xlx_offset_byte_param_output_feature_map_5 = 0*2;
  // Collect __xlx_output_feature_map_6__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_6__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_6__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_6)[i]);
}
  int __xlx_size_param_output_feature_map_6 = 480;
  int __xlx_offset_param_output_feature_map_6 = 0;
  int __xlx_offset_byte_param_output_feature_map_6 = 0*2;
  // Collect __xlx_output_feature_map_7__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_7__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_7__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_7)[i]);
}
  int __xlx_size_param_output_feature_map_7 = 480;
  int __xlx_offset_param_output_feature_map_7 = 0;
  int __xlx_offset_byte_param_output_feature_map_7 = 0*2;
  // Collect __xlx_output_feature_map_8__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_8__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_8__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_8)[i]);
}
  int __xlx_size_param_output_feature_map_8 = 480;
  int __xlx_offset_param_output_feature_map_8 = 0;
  int __xlx_offset_byte_param_output_feature_map_8 = 0*2;
  // Collect __xlx_output_feature_map_9__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_9__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_9__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_9)[i]);
}
  int __xlx_size_param_output_feature_map_9 = 480;
  int __xlx_offset_param_output_feature_map_9 = 0;
  int __xlx_offset_byte_param_output_feature_map_9 = 0*2;
  // Collect __xlx_output_feature_map_10__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_10__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_10__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_10)[i]);
}
  int __xlx_size_param_output_feature_map_10 = 480;
  int __xlx_offset_param_output_feature_map_10 = 0;
  int __xlx_offset_byte_param_output_feature_map_10 = 0*2;
  // Collect __xlx_output_feature_map_11__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_11__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_11__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_11)[i]);
}
  int __xlx_size_param_output_feature_map_11 = 480;
  int __xlx_offset_param_output_feature_map_11 = 0;
  int __xlx_offset_byte_param_output_feature_map_11 = 0*2;
  // Collect __xlx_output_feature_map_12__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_12__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_12__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_12)[i]);
}
  int __xlx_size_param_output_feature_map_12 = 480;
  int __xlx_offset_param_output_feature_map_12 = 0;
  int __xlx_offset_byte_param_output_feature_map_12 = 0*2;
  // Collect __xlx_output_feature_map_13__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_13__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_13__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_13)[i]);
}
  int __xlx_size_param_output_feature_map_13 = 480;
  int __xlx_offset_param_output_feature_map_13 = 0;
  int __xlx_offset_byte_param_output_feature_map_13 = 0*2;
  // Collect __xlx_output_feature_map_14__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_14__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_14__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_14)[i]);
}
  int __xlx_size_param_output_feature_map_14 = 480;
  int __xlx_offset_param_output_feature_map_14 = 0;
  int __xlx_offset_byte_param_output_feature_map_14 = 0*2;
  // Collect __xlx_output_feature_map_15__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_15__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_15__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_15)[i]);
}
  int __xlx_size_param_output_feature_map_15 = 480;
  int __xlx_offset_param_output_feature_map_15 = 0;
  int __xlx_offset_byte_param_output_feature_map_15 = 0*2;
  // Collect __xlx_output_feature_map_16__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_16__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_16__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_16)[i]);
}
  int __xlx_size_param_output_feature_map_16 = 480;
  int __xlx_offset_param_output_feature_map_16 = 0;
  int __xlx_offset_byte_param_output_feature_map_16 = 0*2;
  // Collect __xlx_output_feature_map_17__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_17__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_17__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_17)[i]);
}
  int __xlx_size_param_output_feature_map_17 = 480;
  int __xlx_offset_param_output_feature_map_17 = 0;
  int __xlx_offset_byte_param_output_feature_map_17 = 0*2;
  // Collect __xlx_output_feature_map_18__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_18__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_18__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_18)[i]);
}
  int __xlx_size_param_output_feature_map_18 = 480;
  int __xlx_offset_param_output_feature_map_18 = 0;
  int __xlx_offset_byte_param_output_feature_map_18 = 0*2;
  // Collect __xlx_output_feature_map_19__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_19__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_19__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_19)[i]);
}
  int __xlx_size_param_output_feature_map_19 = 480;
  int __xlx_offset_param_output_feature_map_19 = 0;
  int __xlx_offset_byte_param_output_feature_map_19 = 0*2;
  // Collect __xlx_output_feature_map_20__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_20__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_20__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_20)[i]);
}
  int __xlx_size_param_output_feature_map_20 = 480;
  int __xlx_offset_param_output_feature_map_20 = 0;
  int __xlx_offset_byte_param_output_feature_map_20 = 0*2;
  // Collect __xlx_output_feature_map_21__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_21__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_21__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_21)[i]);
}
  int __xlx_size_param_output_feature_map_21 = 480;
  int __xlx_offset_param_output_feature_map_21 = 0;
  int __xlx_offset_byte_param_output_feature_map_21 = 0*2;
  // Collect __xlx_output_feature_map_22__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_22__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_22__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_22)[i]);
}
  int __xlx_size_param_output_feature_map_22 = 480;
  int __xlx_offset_param_output_feature_map_22 = 0;
  int __xlx_offset_byte_param_output_feature_map_22 = 0*2;
  // Collect __xlx_output_feature_map_23__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_23__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_23__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_23)[i]);
}
  int __xlx_size_param_output_feature_map_23 = 480;
  int __xlx_offset_param_output_feature_map_23 = 0;
  int __xlx_offset_byte_param_output_feature_map_23 = 0*2;
  // Collect __xlx_output_feature_map_24__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_24__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_24__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_24)[i]);
}
  int __xlx_size_param_output_feature_map_24 = 480;
  int __xlx_offset_param_output_feature_map_24 = 0;
  int __xlx_offset_byte_param_output_feature_map_24 = 0*2;
  // Collect __xlx_output_feature_map_25__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_25__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_25__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_25)[i]);
}
  int __xlx_size_param_output_feature_map_25 = 480;
  int __xlx_offset_param_output_feature_map_25 = 0;
  int __xlx_offset_byte_param_output_feature_map_25 = 0*2;
  // Collect __xlx_output_feature_map_26__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_26__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_26__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_26)[i]);
}
  int __xlx_size_param_output_feature_map_26 = 480;
  int __xlx_offset_param_output_feature_map_26 = 0;
  int __xlx_offset_byte_param_output_feature_map_26 = 0*2;
  // Collect __xlx_output_feature_map_27__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_27__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_27__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_27)[i]);
}
  int __xlx_size_param_output_feature_map_27 = 480;
  int __xlx_offset_param_output_feature_map_27 = 0;
  int __xlx_offset_byte_param_output_feature_map_27 = 0*2;
  // Collect __xlx_output_feature_map_28__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_28__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_28__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_28)[i]);
}
  int __xlx_size_param_output_feature_map_28 = 480;
  int __xlx_offset_param_output_feature_map_28 = 0;
  int __xlx_offset_byte_param_output_feature_map_28 = 0*2;
  // Collect __xlx_output_feature_map_29__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_29__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_29__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_29)[i]);
}
  int __xlx_size_param_output_feature_map_29 = 480;
  int __xlx_offset_param_output_feature_map_29 = 0;
  int __xlx_offset_byte_param_output_feature_map_29 = 0*2;
  // Collect __xlx_output_feature_map_30__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_30__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_30__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_30)[i]);
}
  int __xlx_size_param_output_feature_map_30 = 480;
  int __xlx_offset_param_output_feature_map_30 = 0;
  int __xlx_offset_byte_param_output_feature_map_30 = 0*2;
  // Collect __xlx_output_feature_map_31__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_31__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_31__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_31)[i]);
}
  int __xlx_size_param_output_feature_map_31 = 480;
  int __xlx_offset_param_output_feature_map_31 = 0;
  int __xlx_offset_byte_param_output_feature_map_31 = 0*2;
  // Collect __xlx_output_feature_map_32__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_32__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_32__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_32)[i]);
}
  int __xlx_size_param_output_feature_map_32 = 480;
  int __xlx_offset_param_output_feature_map_32 = 0;
  int __xlx_offset_byte_param_output_feature_map_32 = 0*2;
  // Collect __xlx_output_feature_map_33__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_33__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_33__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_33)[i]);
}
  int __xlx_size_param_output_feature_map_33 = 480;
  int __xlx_offset_param_output_feature_map_33 = 0;
  int __xlx_offset_byte_param_output_feature_map_33 = 0*2;
  // Collect __xlx_output_feature_map_34__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_34__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_34__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_34)[i]);
}
  int __xlx_size_param_output_feature_map_34 = 480;
  int __xlx_offset_param_output_feature_map_34 = 0;
  int __xlx_offset_byte_param_output_feature_map_34 = 0*2;
  // Collect __xlx_output_feature_map_35__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_35__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_35__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_35)[i]);
}
  int __xlx_size_param_output_feature_map_35 = 480;
  int __xlx_offset_param_output_feature_map_35 = 0;
  int __xlx_offset_byte_param_output_feature_map_35 = 0*2;
  // Collect __xlx_output_feature_map_36__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_36__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_36__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_36)[i]);
}
  int __xlx_size_param_output_feature_map_36 = 480;
  int __xlx_offset_param_output_feature_map_36 = 0;
  int __xlx_offset_byte_param_output_feature_map_36 = 0*2;
  // Collect __xlx_output_feature_map_37__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_37__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_37__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_37)[i]);
}
  int __xlx_size_param_output_feature_map_37 = 480;
  int __xlx_offset_param_output_feature_map_37 = 0;
  int __xlx_offset_byte_param_output_feature_map_37 = 0*2;
  // Collect __xlx_output_feature_map_38__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_38__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_38__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_38)[i]);
}
  int __xlx_size_param_output_feature_map_38 = 480;
  int __xlx_offset_param_output_feature_map_38 = 0;
  int __xlx_offset_byte_param_output_feature_map_38 = 0*2;
  // Collect __xlx_output_feature_map_39__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_39__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_39__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_39)[i]);
}
  int __xlx_size_param_output_feature_map_39 = 480;
  int __xlx_offset_param_output_feature_map_39 = 0;
  int __xlx_offset_byte_param_output_feature_map_39 = 0*2;
  // Collect __xlx_output_feature_map_40__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_40__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_40__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_40)[i]);
}
  int __xlx_size_param_output_feature_map_40 = 480;
  int __xlx_offset_param_output_feature_map_40 = 0;
  int __xlx_offset_byte_param_output_feature_map_40 = 0*2;
  // Collect __xlx_output_feature_map_41__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_41__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_41__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_41)[i]);
}
  int __xlx_size_param_output_feature_map_41 = 480;
  int __xlx_offset_param_output_feature_map_41 = 0;
  int __xlx_offset_byte_param_output_feature_map_41 = 0*2;
  // Collect __xlx_output_feature_map_42__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_42__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_42__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_42)[i]);
}
  int __xlx_size_param_output_feature_map_42 = 480;
  int __xlx_offset_param_output_feature_map_42 = 0;
  int __xlx_offset_byte_param_output_feature_map_42 = 0*2;
  // Collect __xlx_output_feature_map_43__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_43__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_43__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_43)[i]);
}
  int __xlx_size_param_output_feature_map_43 = 480;
  int __xlx_offset_param_output_feature_map_43 = 0;
  int __xlx_offset_byte_param_output_feature_map_43 = 0*2;
  // Collect __xlx_output_feature_map_44__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_44__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_44__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_44)[i]);
}
  int __xlx_size_param_output_feature_map_44 = 480;
  int __xlx_offset_param_output_feature_map_44 = 0;
  int __xlx_offset_byte_param_output_feature_map_44 = 0*2;
  // Collect __xlx_output_feature_map_45__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_45__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_45__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_45)[i]);
}
  int __xlx_size_param_output_feature_map_45 = 480;
  int __xlx_offset_param_output_feature_map_45 = 0;
  int __xlx_offset_byte_param_output_feature_map_45 = 0*2;
  // Collect __xlx_output_feature_map_46__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_46__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_46__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_46)[i]);
}
  int __xlx_size_param_output_feature_map_46 = 480;
  int __xlx_offset_param_output_feature_map_46 = 0;
  int __xlx_offset_byte_param_output_feature_map_46 = 0*2;
  // Collect __xlx_output_feature_map_47__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_47__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_47__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_47)[i]);
}
  int __xlx_size_param_output_feature_map_47 = 480;
  int __xlx_offset_param_output_feature_map_47 = 0;
  int __xlx_offset_byte_param_output_feature_map_47 = 0*2;
  // Collect __xlx_output_feature_map_48__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_48__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_48__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_48)[i]);
}
  int __xlx_size_param_output_feature_map_48 = 480;
  int __xlx_offset_param_output_feature_map_48 = 0;
  int __xlx_offset_byte_param_output_feature_map_48 = 0*2;
  // Collect __xlx_output_feature_map_49__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_49__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_49__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_49)[i]);
}
  int __xlx_size_param_output_feature_map_49 = 480;
  int __xlx_offset_param_output_feature_map_49 = 0;
  int __xlx_offset_byte_param_output_feature_map_49 = 0*2;
  // Collect __xlx_output_feature_map_50__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_50__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_50__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_50)[i]);
}
  int __xlx_size_param_output_feature_map_50 = 480;
  int __xlx_offset_param_output_feature_map_50 = 0;
  int __xlx_offset_byte_param_output_feature_map_50 = 0*2;
  // Collect __xlx_output_feature_map_51__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_51__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_51__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_51)[i]);
}
  int __xlx_size_param_output_feature_map_51 = 480;
  int __xlx_offset_param_output_feature_map_51 = 0;
  int __xlx_offset_byte_param_output_feature_map_51 = 0*2;
  // Collect __xlx_output_feature_map_52__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_52__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_52__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_52)[i]);
}
  int __xlx_size_param_output_feature_map_52 = 480;
  int __xlx_offset_param_output_feature_map_52 = 0;
  int __xlx_offset_byte_param_output_feature_map_52 = 0*2;
  // Collect __xlx_output_feature_map_53__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_53__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_53__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_53)[i]);
}
  int __xlx_size_param_output_feature_map_53 = 480;
  int __xlx_offset_param_output_feature_map_53 = 0;
  int __xlx_offset_byte_param_output_feature_map_53 = 0*2;
  // Collect __xlx_output_feature_map_54__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_54__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_54__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_54)[i]);
}
  int __xlx_size_param_output_feature_map_54 = 480;
  int __xlx_offset_param_output_feature_map_54 = 0;
  int __xlx_offset_byte_param_output_feature_map_54 = 0*2;
  // Collect __xlx_output_feature_map_55__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_55__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_55__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_55)[i]);
}
  int __xlx_size_param_output_feature_map_55 = 480;
  int __xlx_offset_param_output_feature_map_55 = 0;
  int __xlx_offset_byte_param_output_feature_map_55 = 0*2;
  // Collect __xlx_output_feature_map_56__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_56__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_56__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_56)[i]);
}
  int __xlx_size_param_output_feature_map_56 = 480;
  int __xlx_offset_param_output_feature_map_56 = 0;
  int __xlx_offset_byte_param_output_feature_map_56 = 0*2;
  // Collect __xlx_output_feature_map_57__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_57__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_57__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_57)[i]);
}
  int __xlx_size_param_output_feature_map_57 = 480;
  int __xlx_offset_param_output_feature_map_57 = 0;
  int __xlx_offset_byte_param_output_feature_map_57 = 0*2;
  // Collect __xlx_output_feature_map_58__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_58__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_58__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_58)[i]);
}
  int __xlx_size_param_output_feature_map_58 = 480;
  int __xlx_offset_param_output_feature_map_58 = 0;
  int __xlx_offset_byte_param_output_feature_map_58 = 0*2;
  // Collect __xlx_output_feature_map_59__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_59__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_59__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_59)[i]);
}
  int __xlx_size_param_output_feature_map_59 = 480;
  int __xlx_offset_param_output_feature_map_59 = 0;
  int __xlx_offset_byte_param_output_feature_map_59 = 0*2;
  // Collect __xlx_output_feature_map_60__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_60__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_60__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_60)[i]);
}
  int __xlx_size_param_output_feature_map_60 = 480;
  int __xlx_offset_param_output_feature_map_60 = 0;
  int __xlx_offset_byte_param_output_feature_map_60 = 0*2;
  // Collect __xlx_output_feature_map_61__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_61__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_61__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_61)[i]);
}
  int __xlx_size_param_output_feature_map_61 = 480;
  int __xlx_offset_param_output_feature_map_61 = 0;
  int __xlx_offset_byte_param_output_feature_map_61 = 0*2;
  // Collect __xlx_output_feature_map_62__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_62__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_62__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_62)[i]);
}
  int __xlx_size_param_output_feature_map_62 = 480;
  int __xlx_offset_param_output_feature_map_62 = 0;
  int __xlx_offset_byte_param_output_feature_map_62 = 0*2;
  // Collect __xlx_output_feature_map_63__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_63__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_63__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_63)[i]);
}
  int __xlx_size_param_output_feature_map_63 = 480;
  int __xlx_offset_param_output_feature_map_63 = 0;
  int __xlx_offset_byte_param_output_feature_map_63 = 0*2;
  // Collect __xlx_output_feature_map_64__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_64__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_64__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_64)[i]);
}
  int __xlx_size_param_output_feature_map_64 = 480;
  int __xlx_offset_param_output_feature_map_64 = 0;
  int __xlx_offset_byte_param_output_feature_map_64 = 0*2;
  // Collect __xlx_output_feature_map_65__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_65__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_65__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_65)[i]);
}
  int __xlx_size_param_output_feature_map_65 = 480;
  int __xlx_offset_param_output_feature_map_65 = 0;
  int __xlx_offset_byte_param_output_feature_map_65 = 0*2;
  // Collect __xlx_output_feature_map_66__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_66__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_66__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_66)[i]);
}
  int __xlx_size_param_output_feature_map_66 = 480;
  int __xlx_offset_param_output_feature_map_66 = 0;
  int __xlx_offset_byte_param_output_feature_map_66 = 0*2;
  // Collect __xlx_output_feature_map_67__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_67__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_67__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_67)[i]);
}
  int __xlx_size_param_output_feature_map_67 = 480;
  int __xlx_offset_param_output_feature_map_67 = 0;
  int __xlx_offset_byte_param_output_feature_map_67 = 0*2;
  // Collect __xlx_output_feature_map_68__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_68__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_68__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_68)[i]);
}
  int __xlx_size_param_output_feature_map_68 = 480;
  int __xlx_offset_param_output_feature_map_68 = 0;
  int __xlx_offset_byte_param_output_feature_map_68 = 0*2;
  // Collect __xlx_output_feature_map_69__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_69__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_69__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_69)[i]);
}
  int __xlx_size_param_output_feature_map_69 = 480;
  int __xlx_offset_param_output_feature_map_69 = 0;
  int __xlx_offset_byte_param_output_feature_map_69 = 0*2;
  // Collect __xlx_output_feature_map_70__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_70__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_70__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_70)[i]);
}
  int __xlx_size_param_output_feature_map_70 = 480;
  int __xlx_offset_param_output_feature_map_70 = 0;
  int __xlx_offset_byte_param_output_feature_map_70 = 0*2;
  // Collect __xlx_output_feature_map_71__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_71__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_71__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_71)[i]);
}
  int __xlx_size_param_output_feature_map_71 = 480;
  int __xlx_offset_param_output_feature_map_71 = 0;
  int __xlx_offset_byte_param_output_feature_map_71 = 0*2;
  // Collect __xlx_output_feature_map_72__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_72__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_72__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_72)[i]);
}
  int __xlx_size_param_output_feature_map_72 = 480;
  int __xlx_offset_param_output_feature_map_72 = 0;
  int __xlx_offset_byte_param_output_feature_map_72 = 0*2;
  // Collect __xlx_output_feature_map_73__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_73__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_73__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_73)[i]);
}
  int __xlx_size_param_output_feature_map_73 = 480;
  int __xlx_offset_param_output_feature_map_73 = 0;
  int __xlx_offset_byte_param_output_feature_map_73 = 0*2;
  // Collect __xlx_output_feature_map_74__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_74__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_74__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_74)[i]);
}
  int __xlx_size_param_output_feature_map_74 = 480;
  int __xlx_offset_param_output_feature_map_74 = 0;
  int __xlx_offset_byte_param_output_feature_map_74 = 0*2;
  // Collect __xlx_output_feature_map_75__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_75__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_75__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_75)[i]);
}
  int __xlx_size_param_output_feature_map_75 = 480;
  int __xlx_offset_param_output_feature_map_75 = 0;
  int __xlx_offset_byte_param_output_feature_map_75 = 0*2;
  // Collect __xlx_output_feature_map_76__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_76__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_76__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_76)[i]);
}
  int __xlx_size_param_output_feature_map_76 = 480;
  int __xlx_offset_param_output_feature_map_76 = 0;
  int __xlx_offset_byte_param_output_feature_map_76 = 0*2;
  // Collect __xlx_output_feature_map_77__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_77__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_77__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_77)[i]);
}
  int __xlx_size_param_output_feature_map_77 = 480;
  int __xlx_offset_param_output_feature_map_77 = 0;
  int __xlx_offset_byte_param_output_feature_map_77 = 0*2;
  // Collect __xlx_output_feature_map_78__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_78__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_78__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_78)[i]);
}
  int __xlx_size_param_output_feature_map_78 = 480;
  int __xlx_offset_param_output_feature_map_78 = 0;
  int __xlx_offset_byte_param_output_feature_map_78 = 0*2;
  // Collect __xlx_output_feature_map_79__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_79__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_79__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_79)[i]);
}
  int __xlx_size_param_output_feature_map_79 = 480;
  int __xlx_offset_param_output_feature_map_79 = 0;
  int __xlx_offset_byte_param_output_feature_map_79 = 0*2;
  // Collect __xlx_output_feature_map_80__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_80__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_80__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_80)[i]);
}
  int __xlx_size_param_output_feature_map_80 = 480;
  int __xlx_offset_param_output_feature_map_80 = 0;
  int __xlx_offset_byte_param_output_feature_map_80 = 0*2;
  // Collect __xlx_output_feature_map_81__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_81__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_81__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_81)[i]);
}
  int __xlx_size_param_output_feature_map_81 = 480;
  int __xlx_offset_param_output_feature_map_81 = 0;
  int __xlx_offset_byte_param_output_feature_map_81 = 0*2;
  // Collect __xlx_output_feature_map_82__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_82__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_82__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_82)[i]);
}
  int __xlx_size_param_output_feature_map_82 = 480;
  int __xlx_offset_param_output_feature_map_82 = 0;
  int __xlx_offset_byte_param_output_feature_map_82 = 0*2;
  // Collect __xlx_output_feature_map_83__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_83__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_83__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_83)[i]);
}
  int __xlx_size_param_output_feature_map_83 = 480;
  int __xlx_offset_param_output_feature_map_83 = 0;
  int __xlx_offset_byte_param_output_feature_map_83 = 0*2;
  // Collect __xlx_output_feature_map_84__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_84__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_84__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_84)[i]);
}
  int __xlx_size_param_output_feature_map_84 = 480;
  int __xlx_offset_param_output_feature_map_84 = 0;
  int __xlx_offset_byte_param_output_feature_map_84 = 0*2;
  // Collect __xlx_output_feature_map_85__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_85__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_85__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_85)[i]);
}
  int __xlx_size_param_output_feature_map_85 = 480;
  int __xlx_offset_param_output_feature_map_85 = 0;
  int __xlx_offset_byte_param_output_feature_map_85 = 0*2;
  // Collect __xlx_output_feature_map_86__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_86__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_86__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_86)[i]);
}
  int __xlx_size_param_output_feature_map_86 = 480;
  int __xlx_offset_param_output_feature_map_86 = 0;
  int __xlx_offset_byte_param_output_feature_map_86 = 0*2;
  // Collect __xlx_output_feature_map_87__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_87__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_87__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_87)[i]);
}
  int __xlx_size_param_output_feature_map_87 = 480;
  int __xlx_offset_param_output_feature_map_87 = 0;
  int __xlx_offset_byte_param_output_feature_map_87 = 0*2;
  // Collect __xlx_output_feature_map_88__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_88__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_88__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_88)[i]);
}
  int __xlx_size_param_output_feature_map_88 = 480;
  int __xlx_offset_param_output_feature_map_88 = 0;
  int __xlx_offset_byte_param_output_feature_map_88 = 0*2;
  // Collect __xlx_output_feature_map_89__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_89__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_89__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_89)[i]);
}
  int __xlx_size_param_output_feature_map_89 = 480;
  int __xlx_offset_param_output_feature_map_89 = 0;
  int __xlx_offset_byte_param_output_feature_map_89 = 0*2;
  // Collect __xlx_output_feature_map_90__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_90__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_90__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_90)[i]);
}
  int __xlx_size_param_output_feature_map_90 = 480;
  int __xlx_offset_param_output_feature_map_90 = 0;
  int __xlx_offset_byte_param_output_feature_map_90 = 0*2;
  // Collect __xlx_output_feature_map_91__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_91__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_91__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_91)[i]);
}
  int __xlx_size_param_output_feature_map_91 = 480;
  int __xlx_offset_param_output_feature_map_91 = 0;
  int __xlx_offset_byte_param_output_feature_map_91 = 0*2;
  // Collect __xlx_output_feature_map_92__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_92__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_92__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_92)[i]);
}
  int __xlx_size_param_output_feature_map_92 = 480;
  int __xlx_offset_param_output_feature_map_92 = 0;
  int __xlx_offset_byte_param_output_feature_map_92 = 0*2;
  // Collect __xlx_output_feature_map_93__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_93__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_93__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_93)[i]);
}
  int __xlx_size_param_output_feature_map_93 = 480;
  int __xlx_offset_param_output_feature_map_93 = 0;
  int __xlx_offset_byte_param_output_feature_map_93 = 0*2;
  // Collect __xlx_output_feature_map_94__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_94__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_94__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_94)[i]);
}
  int __xlx_size_param_output_feature_map_94 = 480;
  int __xlx_offset_param_output_feature_map_94 = 0;
  int __xlx_offset_byte_param_output_feature_map_94 = 0*2;
  // Collect __xlx_output_feature_map_95__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_95__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_95__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_95)[i]);
}
  int __xlx_size_param_output_feature_map_95 = 480;
  int __xlx_offset_param_output_feature_map_95 = 0;
  int __xlx_offset_byte_param_output_feature_map_95 = 0*2;
  // Collect __xlx_output_feature_map_96__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_96__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_96__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_96)[i]);
}
  int __xlx_size_param_output_feature_map_96 = 480;
  int __xlx_offset_param_output_feature_map_96 = 0;
  int __xlx_offset_byte_param_output_feature_map_96 = 0*2;
  // Collect __xlx_output_feature_map_97__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_97__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_97__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_97)[i]);
}
  int __xlx_size_param_output_feature_map_97 = 480;
  int __xlx_offset_param_output_feature_map_97 = 0;
  int __xlx_offset_byte_param_output_feature_map_97 = 0*2;
  // Collect __xlx_output_feature_map_98__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_98__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_98__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_98)[i]);
}
  int __xlx_size_param_output_feature_map_98 = 480;
  int __xlx_offset_param_output_feature_map_98 = 0;
  int __xlx_offset_byte_param_output_feature_map_98 = 0*2;
  // Collect __xlx_output_feature_map_99__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_99__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_99__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_99)[i]);
}
  int __xlx_size_param_output_feature_map_99 = 480;
  int __xlx_offset_param_output_feature_map_99 = 0;
  int __xlx_offset_byte_param_output_feature_map_99 = 0*2;
  // Collect __xlx_output_feature_map_100__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_100__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_100__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_100)[i]);
}
  int __xlx_size_param_output_feature_map_100 = 480;
  int __xlx_offset_param_output_feature_map_100 = 0;
  int __xlx_offset_byte_param_output_feature_map_100 = 0*2;
  // Collect __xlx_output_feature_map_101__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_101__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_101__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_101)[i]);
}
  int __xlx_size_param_output_feature_map_101 = 480;
  int __xlx_offset_param_output_feature_map_101 = 0;
  int __xlx_offset_byte_param_output_feature_map_101 = 0*2;
  // Collect __xlx_output_feature_map_102__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_102__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_102__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_102)[i]);
}
  int __xlx_size_param_output_feature_map_102 = 480;
  int __xlx_offset_param_output_feature_map_102 = 0;
  int __xlx_offset_byte_param_output_feature_map_102 = 0*2;
  // Collect __xlx_output_feature_map_103__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_103__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_103__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_103)[i]);
}
  int __xlx_size_param_output_feature_map_103 = 480;
  int __xlx_offset_param_output_feature_map_103 = 0;
  int __xlx_offset_byte_param_output_feature_map_103 = 0*2;
  // Collect __xlx_output_feature_map_104__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_104__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_104__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_104)[i]);
}
  int __xlx_size_param_output_feature_map_104 = 480;
  int __xlx_offset_param_output_feature_map_104 = 0;
  int __xlx_offset_byte_param_output_feature_map_104 = 0*2;
  // Collect __xlx_output_feature_map_105__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_105__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_105__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_105)[i]);
}
  int __xlx_size_param_output_feature_map_105 = 480;
  int __xlx_offset_param_output_feature_map_105 = 0;
  int __xlx_offset_byte_param_output_feature_map_105 = 0*2;
  // Collect __xlx_output_feature_map_106__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_106__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_106__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_106)[i]);
}
  int __xlx_size_param_output_feature_map_106 = 480;
  int __xlx_offset_param_output_feature_map_106 = 0;
  int __xlx_offset_byte_param_output_feature_map_106 = 0*2;
  // Collect __xlx_output_feature_map_107__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_107__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_107__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_107)[i]);
}
  int __xlx_size_param_output_feature_map_107 = 480;
  int __xlx_offset_param_output_feature_map_107 = 0;
  int __xlx_offset_byte_param_output_feature_map_107 = 0*2;
  // Collect __xlx_output_feature_map_108__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_108__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_108__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_108)[i]);
}
  int __xlx_size_param_output_feature_map_108 = 480;
  int __xlx_offset_param_output_feature_map_108 = 0;
  int __xlx_offset_byte_param_output_feature_map_108 = 0*2;
  // Collect __xlx_output_feature_map_109__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_109__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_109__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_109)[i]);
}
  int __xlx_size_param_output_feature_map_109 = 480;
  int __xlx_offset_param_output_feature_map_109 = 0;
  int __xlx_offset_byte_param_output_feature_map_109 = 0*2;
  // Collect __xlx_output_feature_map_110__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_110__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_110__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_110)[i]);
}
  int __xlx_size_param_output_feature_map_110 = 480;
  int __xlx_offset_param_output_feature_map_110 = 0;
  int __xlx_offset_byte_param_output_feature_map_110 = 0*2;
  // Collect __xlx_output_feature_map_111__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_111__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_111__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_111)[i]);
}
  int __xlx_size_param_output_feature_map_111 = 480;
  int __xlx_offset_param_output_feature_map_111 = 0;
  int __xlx_offset_byte_param_output_feature_map_111 = 0*2;
  // Collect __xlx_output_feature_map_112__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_112__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_112__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_112)[i]);
}
  int __xlx_size_param_output_feature_map_112 = 480;
  int __xlx_offset_param_output_feature_map_112 = 0;
  int __xlx_offset_byte_param_output_feature_map_112 = 0*2;
  // Collect __xlx_output_feature_map_113__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_113__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_113__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_113)[i]);
}
  int __xlx_size_param_output_feature_map_113 = 480;
  int __xlx_offset_param_output_feature_map_113 = 0;
  int __xlx_offset_byte_param_output_feature_map_113 = 0*2;
  // Collect __xlx_output_feature_map_114__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_114__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_114__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_114)[i]);
}
  int __xlx_size_param_output_feature_map_114 = 480;
  int __xlx_offset_param_output_feature_map_114 = 0;
  int __xlx_offset_byte_param_output_feature_map_114 = 0*2;
  // Collect __xlx_output_feature_map_115__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_115__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_115__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_115)[i]);
}
  int __xlx_size_param_output_feature_map_115 = 480;
  int __xlx_offset_param_output_feature_map_115 = 0;
  int __xlx_offset_byte_param_output_feature_map_115 = 0*2;
  // Collect __xlx_output_feature_map_116__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_116__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_116__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_116)[i]);
}
  int __xlx_size_param_output_feature_map_116 = 480;
  int __xlx_offset_param_output_feature_map_116 = 0;
  int __xlx_offset_byte_param_output_feature_map_116 = 0*2;
  // Collect __xlx_output_feature_map_117__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_117__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_117__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_117)[i]);
}
  int __xlx_size_param_output_feature_map_117 = 480;
  int __xlx_offset_param_output_feature_map_117 = 0;
  int __xlx_offset_byte_param_output_feature_map_117 = 0*2;
  // Collect __xlx_output_feature_map_118__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_118__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_118__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_118)[i]);
}
  int __xlx_size_param_output_feature_map_118 = 480;
  int __xlx_offset_param_output_feature_map_118 = 0;
  int __xlx_offset_byte_param_output_feature_map_118 = 0*2;
  // Collect __xlx_output_feature_map_119__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_119__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_119__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_119)[i]);
}
  int __xlx_size_param_output_feature_map_119 = 480;
  int __xlx_offset_param_output_feature_map_119 = 0;
  int __xlx_offset_byte_param_output_feature_map_119 = 0*2;
  // Collect __xlx_output_feature_map_120__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_120__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_120__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_120)[i]);
}
  int __xlx_size_param_output_feature_map_120 = 480;
  int __xlx_offset_param_output_feature_map_120 = 0;
  int __xlx_offset_byte_param_output_feature_map_120 = 0*2;
  // Collect __xlx_output_feature_map_121__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_121__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_121__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_121)[i]);
}
  int __xlx_size_param_output_feature_map_121 = 480;
  int __xlx_offset_param_output_feature_map_121 = 0;
  int __xlx_offset_byte_param_output_feature_map_121 = 0*2;
  // Collect __xlx_output_feature_map_122__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_122__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_122__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_122)[i]);
}
  int __xlx_size_param_output_feature_map_122 = 480;
  int __xlx_offset_param_output_feature_map_122 = 0;
  int __xlx_offset_byte_param_output_feature_map_122 = 0*2;
  // Collect __xlx_output_feature_map_123__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_123__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_123__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_123)[i]);
}
  int __xlx_size_param_output_feature_map_123 = 480;
  int __xlx_offset_param_output_feature_map_123 = 0;
  int __xlx_offset_byte_param_output_feature_map_123 = 0*2;
  // Collect __xlx_output_feature_map_124__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_124__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_124__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_124)[i]);
}
  int __xlx_size_param_output_feature_map_124 = 480;
  int __xlx_offset_param_output_feature_map_124 = 0;
  int __xlx_offset_byte_param_output_feature_map_124 = 0*2;
  // Collect __xlx_output_feature_map_125__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_125__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_125__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_125)[i]);
}
  int __xlx_size_param_output_feature_map_125 = 480;
  int __xlx_offset_param_output_feature_map_125 = 0;
  int __xlx_offset_byte_param_output_feature_map_125 = 0*2;
  // Collect __xlx_output_feature_map_126__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_126__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_126__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_126)[i]);
}
  int __xlx_size_param_output_feature_map_126 = 480;
  int __xlx_offset_param_output_feature_map_126 = 0;
  int __xlx_offset_byte_param_output_feature_map_126 = 0*2;
  // Collect __xlx_output_feature_map_127__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_127__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_127__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_127)[i]);
}
  int __xlx_size_param_output_feature_map_127 = 480;
  int __xlx_offset_param_output_feature_map_127 = 0;
  int __xlx_offset_byte_param_output_feature_map_127 = 0*2;
  // Collect __xlx_output_feature_map_128__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_128__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_128__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_128)[i]);
}
  int __xlx_size_param_output_feature_map_128 = 480;
  int __xlx_offset_param_output_feature_map_128 = 0;
  int __xlx_offset_byte_param_output_feature_map_128 = 0*2;
  // Collect __xlx_output_feature_map_129__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_129__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_129__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_129)[i]);
}
  int __xlx_size_param_output_feature_map_129 = 480;
  int __xlx_offset_param_output_feature_map_129 = 0;
  int __xlx_offset_byte_param_output_feature_map_129 = 0*2;
  // Collect __xlx_output_feature_map_130__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_130__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_130__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_130)[i]);
}
  int __xlx_size_param_output_feature_map_130 = 480;
  int __xlx_offset_param_output_feature_map_130 = 0;
  int __xlx_offset_byte_param_output_feature_map_130 = 0*2;
  // Collect __xlx_output_feature_map_131__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_131__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_131__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_131)[i]);
}
  int __xlx_size_param_output_feature_map_131 = 480;
  int __xlx_offset_param_output_feature_map_131 = 0;
  int __xlx_offset_byte_param_output_feature_map_131 = 0*2;
  // Collect __xlx_output_feature_map_132__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_132__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_132__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_132)[i]);
}
  int __xlx_size_param_output_feature_map_132 = 480;
  int __xlx_offset_param_output_feature_map_132 = 0;
  int __xlx_offset_byte_param_output_feature_map_132 = 0*2;
  // Collect __xlx_output_feature_map_133__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_133__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_133__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_133)[i]);
}
  int __xlx_size_param_output_feature_map_133 = 480;
  int __xlx_offset_param_output_feature_map_133 = 0;
  int __xlx_offset_byte_param_output_feature_map_133 = 0*2;
  // Collect __xlx_output_feature_map_134__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_134__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_134__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_134)[i]);
}
  int __xlx_size_param_output_feature_map_134 = 480;
  int __xlx_offset_param_output_feature_map_134 = 0;
  int __xlx_offset_byte_param_output_feature_map_134 = 0*2;
  // Collect __xlx_output_feature_map_135__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_135__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_135__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_135)[i]);
}
  int __xlx_size_param_output_feature_map_135 = 480;
  int __xlx_offset_param_output_feature_map_135 = 0;
  int __xlx_offset_byte_param_output_feature_map_135 = 0*2;
  // Collect __xlx_output_feature_map_136__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_136__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_136__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_136)[i]);
}
  int __xlx_size_param_output_feature_map_136 = 480;
  int __xlx_offset_param_output_feature_map_136 = 0;
  int __xlx_offset_byte_param_output_feature_map_136 = 0*2;
  // Collect __xlx_output_feature_map_137__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_137__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_137__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_137)[i]);
}
  int __xlx_size_param_output_feature_map_137 = 480;
  int __xlx_offset_param_output_feature_map_137 = 0;
  int __xlx_offset_byte_param_output_feature_map_137 = 0*2;
  // Collect __xlx_output_feature_map_138__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_138__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_138__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_138)[i]);
}
  int __xlx_size_param_output_feature_map_138 = 480;
  int __xlx_offset_param_output_feature_map_138 = 0;
  int __xlx_offset_byte_param_output_feature_map_138 = 0*2;
  // Collect __xlx_output_feature_map_139__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_139__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_139__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_139)[i]);
}
  int __xlx_size_param_output_feature_map_139 = 480;
  int __xlx_offset_param_output_feature_map_139 = 0;
  int __xlx_offset_byte_param_output_feature_map_139 = 0*2;
  // Collect __xlx_output_feature_map_140__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_140__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_140__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_140)[i]);
}
  int __xlx_size_param_output_feature_map_140 = 480;
  int __xlx_offset_param_output_feature_map_140 = 0;
  int __xlx_offset_byte_param_output_feature_map_140 = 0*2;
  // Collect __xlx_output_feature_map_141__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_141__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_141__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_141)[i]);
}
  int __xlx_size_param_output_feature_map_141 = 480;
  int __xlx_offset_param_output_feature_map_141 = 0;
  int __xlx_offset_byte_param_output_feature_map_141 = 0*2;
  // Collect __xlx_output_feature_map_142__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_142__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_142__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_142)[i]);
}
  int __xlx_size_param_output_feature_map_142 = 480;
  int __xlx_offset_param_output_feature_map_142 = 0;
  int __xlx_offset_byte_param_output_feature_map_142 = 0*2;
  // Collect __xlx_output_feature_map_143__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_143__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_143__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_143)[i]);
}
  int __xlx_size_param_output_feature_map_143 = 480;
  int __xlx_offset_param_output_feature_map_143 = 0;
  int __xlx_offset_byte_param_output_feature_map_143 = 0*2;
  // Collect __xlx_output_feature_map_144__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_144__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_144__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_144)[i]);
}
  int __xlx_size_param_output_feature_map_144 = 480;
  int __xlx_offset_param_output_feature_map_144 = 0;
  int __xlx_offset_byte_param_output_feature_map_144 = 0*2;
  // Collect __xlx_output_feature_map_145__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_145__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_145__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_145)[i]);
}
  int __xlx_size_param_output_feature_map_145 = 480;
  int __xlx_offset_param_output_feature_map_145 = 0;
  int __xlx_offset_byte_param_output_feature_map_145 = 0*2;
  // Collect __xlx_output_feature_map_146__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_146__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_146__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_146)[i]);
}
  int __xlx_size_param_output_feature_map_146 = 480;
  int __xlx_offset_param_output_feature_map_146 = 0;
  int __xlx_offset_byte_param_output_feature_map_146 = 0*2;
  // Collect __xlx_output_feature_map_147__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_147__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_147__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_147)[i]);
}
  int __xlx_size_param_output_feature_map_147 = 480;
  int __xlx_offset_param_output_feature_map_147 = 0;
  int __xlx_offset_byte_param_output_feature_map_147 = 0*2;
  // Collect __xlx_output_feature_map_148__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_148__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_148__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_148)[i]);
}
  int __xlx_size_param_output_feature_map_148 = 480;
  int __xlx_offset_param_output_feature_map_148 = 0;
  int __xlx_offset_byte_param_output_feature_map_148 = 0*2;
  // Collect __xlx_output_feature_map_149__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_149__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_149__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_149)[i]);
}
  int __xlx_size_param_output_feature_map_149 = 480;
  int __xlx_offset_param_output_feature_map_149 = 0;
  int __xlx_offset_byte_param_output_feature_map_149 = 0*2;
  // Collect __xlx_output_feature_map_150__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_150__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_150__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_150)[i]);
}
  int __xlx_size_param_output_feature_map_150 = 480;
  int __xlx_offset_param_output_feature_map_150 = 0;
  int __xlx_offset_byte_param_output_feature_map_150 = 0*2;
  // Collect __xlx_output_feature_map_151__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_151__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_151__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_151)[i]);
}
  int __xlx_size_param_output_feature_map_151 = 480;
  int __xlx_offset_param_output_feature_map_151 = 0;
  int __xlx_offset_byte_param_output_feature_map_151 = 0*2;
  // Collect __xlx_output_feature_map_152__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_152__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_152__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_152)[i]);
}
  int __xlx_size_param_output_feature_map_152 = 480;
  int __xlx_offset_param_output_feature_map_152 = 0;
  int __xlx_offset_byte_param_output_feature_map_152 = 0*2;
  // Collect __xlx_output_feature_map_153__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_153__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_153__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_153)[i]);
}
  int __xlx_size_param_output_feature_map_153 = 480;
  int __xlx_offset_param_output_feature_map_153 = 0;
  int __xlx_offset_byte_param_output_feature_map_153 = 0*2;
  // Collect __xlx_output_feature_map_154__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_154__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_154__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_154)[i]);
}
  int __xlx_size_param_output_feature_map_154 = 480;
  int __xlx_offset_param_output_feature_map_154 = 0;
  int __xlx_offset_byte_param_output_feature_map_154 = 0*2;
  // Collect __xlx_output_feature_map_155__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_155__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_155__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_155)[i]);
}
  int __xlx_size_param_output_feature_map_155 = 480;
  int __xlx_offset_param_output_feature_map_155 = 0;
  int __xlx_offset_byte_param_output_feature_map_155 = 0*2;
  // Collect __xlx_output_feature_map_156__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_156__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_156__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_156)[i]);
}
  int __xlx_size_param_output_feature_map_156 = 480;
  int __xlx_offset_param_output_feature_map_156 = 0;
  int __xlx_offset_byte_param_output_feature_map_156 = 0*2;
  // Collect __xlx_output_feature_map_157__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_157__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_157__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_157)[i]);
}
  int __xlx_size_param_output_feature_map_157 = 480;
  int __xlx_offset_param_output_feature_map_157 = 0;
  int __xlx_offset_byte_param_output_feature_map_157 = 0*2;
  // Collect __xlx_output_feature_map_158__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_158__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_158__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_158)[i]);
}
  int __xlx_size_param_output_feature_map_158 = 480;
  int __xlx_offset_param_output_feature_map_158 = 0;
  int __xlx_offset_byte_param_output_feature_map_158 = 0*2;
  // Collect __xlx_output_feature_map_159__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_159__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_159__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_159)[i]);
}
  int __xlx_size_param_output_feature_map_159 = 480;
  int __xlx_offset_param_output_feature_map_159 = 0;
  int __xlx_offset_byte_param_output_feature_map_159 = 0*2;
  // Collect __xlx_output_feature_map_160__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_160__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_160__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_160)[i]);
}
  int __xlx_size_param_output_feature_map_160 = 480;
  int __xlx_offset_param_output_feature_map_160 = 0;
  int __xlx_offset_byte_param_output_feature_map_160 = 0*2;
  // Collect __xlx_output_feature_map_161__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_161__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_161__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_161)[i]);
}
  int __xlx_size_param_output_feature_map_161 = 480;
  int __xlx_offset_param_output_feature_map_161 = 0;
  int __xlx_offset_byte_param_output_feature_map_161 = 0*2;
  // Collect __xlx_output_feature_map_162__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_162__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_162__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_162)[i]);
}
  int __xlx_size_param_output_feature_map_162 = 480;
  int __xlx_offset_param_output_feature_map_162 = 0;
  int __xlx_offset_byte_param_output_feature_map_162 = 0*2;
  // Collect __xlx_output_feature_map_163__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_163__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_163__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_163)[i]);
}
  int __xlx_size_param_output_feature_map_163 = 480;
  int __xlx_offset_param_output_feature_map_163 = 0;
  int __xlx_offset_byte_param_output_feature_map_163 = 0*2;
  // Collect __xlx_output_feature_map_164__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_164__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_164__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_164)[i]);
}
  int __xlx_size_param_output_feature_map_164 = 480;
  int __xlx_offset_param_output_feature_map_164 = 0;
  int __xlx_offset_byte_param_output_feature_map_164 = 0*2;
  // Collect __xlx_output_feature_map_165__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_165__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_165__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_165)[i]);
}
  int __xlx_size_param_output_feature_map_165 = 480;
  int __xlx_offset_param_output_feature_map_165 = 0;
  int __xlx_offset_byte_param_output_feature_map_165 = 0*2;
  // Collect __xlx_output_feature_map_166__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_166__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_166__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_166)[i]);
}
  int __xlx_size_param_output_feature_map_166 = 480;
  int __xlx_offset_param_output_feature_map_166 = 0;
  int __xlx_offset_byte_param_output_feature_map_166 = 0*2;
  // Collect __xlx_output_feature_map_167__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_167__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_167__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_167)[i]);
}
  int __xlx_size_param_output_feature_map_167 = 480;
  int __xlx_offset_param_output_feature_map_167 = 0;
  int __xlx_offset_byte_param_output_feature_map_167 = 0*2;
  // Collect __xlx_output_feature_map_168__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_168__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_168__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_168)[i]);
}
  int __xlx_size_param_output_feature_map_168 = 480;
  int __xlx_offset_param_output_feature_map_168 = 0;
  int __xlx_offset_byte_param_output_feature_map_168 = 0*2;
  // Collect __xlx_output_feature_map_169__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_169__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_169__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_169)[i]);
}
  int __xlx_size_param_output_feature_map_169 = 480;
  int __xlx_offset_param_output_feature_map_169 = 0;
  int __xlx_offset_byte_param_output_feature_map_169 = 0*2;
  // Collect __xlx_output_feature_map_170__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_170__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_170__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_170)[i]);
}
  int __xlx_size_param_output_feature_map_170 = 480;
  int __xlx_offset_param_output_feature_map_170 = 0;
  int __xlx_offset_byte_param_output_feature_map_170 = 0*2;
  // Collect __xlx_output_feature_map_171__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_171__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_171__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_171)[i]);
}
  int __xlx_size_param_output_feature_map_171 = 480;
  int __xlx_offset_param_output_feature_map_171 = 0;
  int __xlx_offset_byte_param_output_feature_map_171 = 0*2;
  // Collect __xlx_output_feature_map_172__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_172__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_172__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_172)[i]);
}
  int __xlx_size_param_output_feature_map_172 = 480;
  int __xlx_offset_param_output_feature_map_172 = 0;
  int __xlx_offset_byte_param_output_feature_map_172 = 0*2;
  // Collect __xlx_output_feature_map_173__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_173__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_173__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_173)[i]);
}
  int __xlx_size_param_output_feature_map_173 = 480;
  int __xlx_offset_param_output_feature_map_173 = 0;
  int __xlx_offset_byte_param_output_feature_map_173 = 0*2;
  // Collect __xlx_output_feature_map_174__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_174__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_174__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_174)[i]);
}
  int __xlx_size_param_output_feature_map_174 = 480;
  int __xlx_offset_param_output_feature_map_174 = 0;
  int __xlx_offset_byte_param_output_feature_map_174 = 0*2;
  // Collect __xlx_output_feature_map_175__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_175__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_175__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_175)[i]);
}
  int __xlx_size_param_output_feature_map_175 = 480;
  int __xlx_offset_param_output_feature_map_175 = 0;
  int __xlx_offset_byte_param_output_feature_map_175 = 0*2;
  // Collect __xlx_output_feature_map_176__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_176__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_176__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_176)[i]);
}
  int __xlx_size_param_output_feature_map_176 = 480;
  int __xlx_offset_param_output_feature_map_176 = 0;
  int __xlx_offset_byte_param_output_feature_map_176 = 0*2;
  // Collect __xlx_output_feature_map_177__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_177__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_177__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_177)[i]);
}
  int __xlx_size_param_output_feature_map_177 = 480;
  int __xlx_offset_param_output_feature_map_177 = 0;
  int __xlx_offset_byte_param_output_feature_map_177 = 0*2;
  // Collect __xlx_output_feature_map_178__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_178__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_178__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_178)[i]);
}
  int __xlx_size_param_output_feature_map_178 = 480;
  int __xlx_offset_param_output_feature_map_178 = 0;
  int __xlx_offset_byte_param_output_feature_map_178 = 0*2;
  // Collect __xlx_output_feature_map_179__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_179__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_179__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_179)[i]);
}
  int __xlx_size_param_output_feature_map_179 = 480;
  int __xlx_offset_param_output_feature_map_179 = 0;
  int __xlx_offset_byte_param_output_feature_map_179 = 0*2;
  // Collect __xlx_output_feature_map_180__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_180__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_180__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_180)[i]);
}
  int __xlx_size_param_output_feature_map_180 = 480;
  int __xlx_offset_param_output_feature_map_180 = 0;
  int __xlx_offset_byte_param_output_feature_map_180 = 0*2;
  // Collect __xlx_output_feature_map_181__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_181__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_181__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_181)[i]);
}
  int __xlx_size_param_output_feature_map_181 = 480;
  int __xlx_offset_param_output_feature_map_181 = 0;
  int __xlx_offset_byte_param_output_feature_map_181 = 0*2;
  // Collect __xlx_output_feature_map_182__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_182__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_182__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_182)[i]);
}
  int __xlx_size_param_output_feature_map_182 = 480;
  int __xlx_offset_param_output_feature_map_182 = 0;
  int __xlx_offset_byte_param_output_feature_map_182 = 0*2;
  // Collect __xlx_output_feature_map_183__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_183__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_183__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_183)[i]);
}
  int __xlx_size_param_output_feature_map_183 = 480;
  int __xlx_offset_param_output_feature_map_183 = 0;
  int __xlx_offset_byte_param_output_feature_map_183 = 0*2;
  // Collect __xlx_output_feature_map_184__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_184__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_184__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_184)[i]);
}
  int __xlx_size_param_output_feature_map_184 = 480;
  int __xlx_offset_param_output_feature_map_184 = 0;
  int __xlx_offset_byte_param_output_feature_map_184 = 0*2;
  // Collect __xlx_output_feature_map_185__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_185__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_185__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_185)[i]);
}
  int __xlx_size_param_output_feature_map_185 = 480;
  int __xlx_offset_param_output_feature_map_185 = 0;
  int __xlx_offset_byte_param_output_feature_map_185 = 0*2;
  // Collect __xlx_output_feature_map_186__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_186__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_186__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_186)[i]);
}
  int __xlx_size_param_output_feature_map_186 = 480;
  int __xlx_offset_param_output_feature_map_186 = 0;
  int __xlx_offset_byte_param_output_feature_map_186 = 0*2;
  // Collect __xlx_output_feature_map_187__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_187__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_187__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_187)[i]);
}
  int __xlx_size_param_output_feature_map_187 = 480;
  int __xlx_offset_param_output_feature_map_187 = 0;
  int __xlx_offset_byte_param_output_feature_map_187 = 0*2;
  // Collect __xlx_output_feature_map_188__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_188__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_188__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_188)[i]);
}
  int __xlx_size_param_output_feature_map_188 = 480;
  int __xlx_offset_param_output_feature_map_188 = 0;
  int __xlx_offset_byte_param_output_feature_map_188 = 0*2;
  // Collect __xlx_output_feature_map_189__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_189__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_189__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_189)[i]);
}
  int __xlx_size_param_output_feature_map_189 = 480;
  int __xlx_offset_param_output_feature_map_189 = 0;
  int __xlx_offset_byte_param_output_feature_map_189 = 0*2;
  // Collect __xlx_output_feature_map_190__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_190__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_190__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_190)[i]);
}
  int __xlx_size_param_output_feature_map_190 = 480;
  int __xlx_offset_param_output_feature_map_190 = 0;
  int __xlx_offset_byte_param_output_feature_map_190 = 0*2;
  // Collect __xlx_output_feature_map_191__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_191__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_191__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_191)[i]);
}
  int __xlx_size_param_output_feature_map_191 = 480;
  int __xlx_offset_param_output_feature_map_191 = 0;
  int __xlx_offset_byte_param_output_feature_map_191 = 0*2;
  // Collect __xlx_output_feature_map_192__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_192__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_192__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_192)[i]);
}
  int __xlx_size_param_output_feature_map_192 = 480;
  int __xlx_offset_param_output_feature_map_192 = 0;
  int __xlx_offset_byte_param_output_feature_map_192 = 0*2;
  // Collect __xlx_output_feature_map_193__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_193__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_193__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_193)[i]);
}
  int __xlx_size_param_output_feature_map_193 = 480;
  int __xlx_offset_param_output_feature_map_193 = 0;
  int __xlx_offset_byte_param_output_feature_map_193 = 0*2;
  // Collect __xlx_output_feature_map_194__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_194__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_194__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_194)[i]);
}
  int __xlx_size_param_output_feature_map_194 = 480;
  int __xlx_offset_param_output_feature_map_194 = 0;
  int __xlx_offset_byte_param_output_feature_map_194 = 0*2;
  // Collect __xlx_output_feature_map_195__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_195__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_195__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_195)[i]);
}
  int __xlx_size_param_output_feature_map_195 = 480;
  int __xlx_offset_param_output_feature_map_195 = 0;
  int __xlx_offset_byte_param_output_feature_map_195 = 0*2;
  // Collect __xlx_output_feature_map_196__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_196__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_196__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_196)[i]);
}
  int __xlx_size_param_output_feature_map_196 = 480;
  int __xlx_offset_param_output_feature_map_196 = 0;
  int __xlx_offset_byte_param_output_feature_map_196 = 0*2;
  // Collect __xlx_output_feature_map_197__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_197__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_197__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_197)[i]);
}
  int __xlx_size_param_output_feature_map_197 = 480;
  int __xlx_offset_param_output_feature_map_197 = 0;
  int __xlx_offset_byte_param_output_feature_map_197 = 0*2;
  // Collect __xlx_output_feature_map_198__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_198__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_198__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_198)[i]);
}
  int __xlx_size_param_output_feature_map_198 = 480;
  int __xlx_offset_param_output_feature_map_198 = 0;
  int __xlx_offset_byte_param_output_feature_map_198 = 0*2;
  // Collect __xlx_output_feature_map_199__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_199__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_199__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_199)[i]);
}
  int __xlx_size_param_output_feature_map_199 = 480;
  int __xlx_offset_param_output_feature_map_199 = 0;
  int __xlx_offset_byte_param_output_feature_map_199 = 0*2;
  // Collect __xlx_output_feature_map_200__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_200__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_200__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_200)[i]);
}
  int __xlx_size_param_output_feature_map_200 = 480;
  int __xlx_offset_param_output_feature_map_200 = 0;
  int __xlx_offset_byte_param_output_feature_map_200 = 0*2;
  // Collect __xlx_output_feature_map_201__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_201__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_201__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_201)[i]);
}
  int __xlx_size_param_output_feature_map_201 = 480;
  int __xlx_offset_param_output_feature_map_201 = 0;
  int __xlx_offset_byte_param_output_feature_map_201 = 0*2;
  // Collect __xlx_output_feature_map_202__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_202__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_202__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_202)[i]);
}
  int __xlx_size_param_output_feature_map_202 = 480;
  int __xlx_offset_param_output_feature_map_202 = 0;
  int __xlx_offset_byte_param_output_feature_map_202 = 0*2;
  // Collect __xlx_output_feature_map_203__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_203__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_203__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_203)[i]);
}
  int __xlx_size_param_output_feature_map_203 = 480;
  int __xlx_offset_param_output_feature_map_203 = 0;
  int __xlx_offset_byte_param_output_feature_map_203 = 0*2;
  // Collect __xlx_output_feature_map_204__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_204__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_204__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_204)[i]);
}
  int __xlx_size_param_output_feature_map_204 = 480;
  int __xlx_offset_param_output_feature_map_204 = 0;
  int __xlx_offset_byte_param_output_feature_map_204 = 0*2;
  // Collect __xlx_output_feature_map_205__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_205__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_205__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_205)[i]);
}
  int __xlx_size_param_output_feature_map_205 = 480;
  int __xlx_offset_param_output_feature_map_205 = 0;
  int __xlx_offset_byte_param_output_feature_map_205 = 0*2;
  // Collect __xlx_output_feature_map_206__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_206__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_206__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_206)[i]);
}
  int __xlx_size_param_output_feature_map_206 = 480;
  int __xlx_offset_param_output_feature_map_206 = 0;
  int __xlx_offset_byte_param_output_feature_map_206 = 0*2;
  // Collect __xlx_output_feature_map_207__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_207__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_207__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_207)[i]);
}
  int __xlx_size_param_output_feature_map_207 = 480;
  int __xlx_offset_param_output_feature_map_207 = 0;
  int __xlx_offset_byte_param_output_feature_map_207 = 0*2;
  // Collect __xlx_output_feature_map_208__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_208__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_208__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_208)[i]);
}
  int __xlx_size_param_output_feature_map_208 = 480;
  int __xlx_offset_param_output_feature_map_208 = 0;
  int __xlx_offset_byte_param_output_feature_map_208 = 0*2;
  // Collect __xlx_output_feature_map_209__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_209__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_209__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_209)[i]);
}
  int __xlx_size_param_output_feature_map_209 = 480;
  int __xlx_offset_param_output_feature_map_209 = 0;
  int __xlx_offset_byte_param_output_feature_map_209 = 0*2;
  // Collect __xlx_output_feature_map_210__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_210__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_210__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_210)[i]);
}
  int __xlx_size_param_output_feature_map_210 = 480;
  int __xlx_offset_param_output_feature_map_210 = 0;
  int __xlx_offset_byte_param_output_feature_map_210 = 0*2;
  // Collect __xlx_output_feature_map_211__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_211__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_211__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_211)[i]);
}
  int __xlx_size_param_output_feature_map_211 = 480;
  int __xlx_offset_param_output_feature_map_211 = 0;
  int __xlx_offset_byte_param_output_feature_map_211 = 0*2;
  // Collect __xlx_output_feature_map_212__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_212__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_212__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_212)[i]);
}
  int __xlx_size_param_output_feature_map_212 = 480;
  int __xlx_offset_param_output_feature_map_212 = 0;
  int __xlx_offset_byte_param_output_feature_map_212 = 0*2;
  // Collect __xlx_output_feature_map_213__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_213__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_213__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_213)[i]);
}
  int __xlx_size_param_output_feature_map_213 = 480;
  int __xlx_offset_param_output_feature_map_213 = 0;
  int __xlx_offset_byte_param_output_feature_map_213 = 0*2;
  // Collect __xlx_output_feature_map_214__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_214__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_214__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_214)[i]);
}
  int __xlx_size_param_output_feature_map_214 = 480;
  int __xlx_offset_param_output_feature_map_214 = 0;
  int __xlx_offset_byte_param_output_feature_map_214 = 0*2;
  // Collect __xlx_output_feature_map_215__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_215__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_215__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_215)[i]);
}
  int __xlx_size_param_output_feature_map_215 = 480;
  int __xlx_offset_param_output_feature_map_215 = 0;
  int __xlx_offset_byte_param_output_feature_map_215 = 0*2;
  // Collect __xlx_output_feature_map_216__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_216__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_216__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_216)[i]);
}
  int __xlx_size_param_output_feature_map_216 = 480;
  int __xlx_offset_param_output_feature_map_216 = 0;
  int __xlx_offset_byte_param_output_feature_map_216 = 0*2;
  // Collect __xlx_output_feature_map_217__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_217__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_217__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_217)[i]);
}
  int __xlx_size_param_output_feature_map_217 = 480;
  int __xlx_offset_param_output_feature_map_217 = 0;
  int __xlx_offset_byte_param_output_feature_map_217 = 0*2;
  // Collect __xlx_output_feature_map_218__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_218__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_218__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_218)[i]);
}
  int __xlx_size_param_output_feature_map_218 = 480;
  int __xlx_offset_param_output_feature_map_218 = 0;
  int __xlx_offset_byte_param_output_feature_map_218 = 0*2;
  // Collect __xlx_output_feature_map_219__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_219__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_219__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_219)[i]);
}
  int __xlx_size_param_output_feature_map_219 = 480;
  int __xlx_offset_param_output_feature_map_219 = 0;
  int __xlx_offset_byte_param_output_feature_map_219 = 0*2;
  // Collect __xlx_output_feature_map_220__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_220__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_220__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_220)[i]);
}
  int __xlx_size_param_output_feature_map_220 = 480;
  int __xlx_offset_param_output_feature_map_220 = 0;
  int __xlx_offset_byte_param_output_feature_map_220 = 0*2;
  // Collect __xlx_output_feature_map_221__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_221__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_221__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_221)[i]);
}
  int __xlx_size_param_output_feature_map_221 = 480;
  int __xlx_offset_param_output_feature_map_221 = 0;
  int __xlx_offset_byte_param_output_feature_map_221 = 0*2;
  // Collect __xlx_output_feature_map_222__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_222__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_222__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_222)[i]);
}
  int __xlx_size_param_output_feature_map_222 = 480;
  int __xlx_offset_param_output_feature_map_222 = 0;
  int __xlx_offset_byte_param_output_feature_map_222 = 0*2;
  // Collect __xlx_output_feature_map_223__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_223__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_223__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_223)[i]);
}
  int __xlx_size_param_output_feature_map_223 = 480;
  int __xlx_offset_param_output_feature_map_223 = 0;
  int __xlx_offset_byte_param_output_feature_map_223 = 0*2;
  // Collect __xlx_output_feature_map_224__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_224__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_224__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_224)[i]);
}
  int __xlx_size_param_output_feature_map_224 = 480;
  int __xlx_offset_param_output_feature_map_224 = 0;
  int __xlx_offset_byte_param_output_feature_map_224 = 0*2;
  // Collect __xlx_output_feature_map_225__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_225__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_225__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_225)[i]);
}
  int __xlx_size_param_output_feature_map_225 = 480;
  int __xlx_offset_param_output_feature_map_225 = 0;
  int __xlx_offset_byte_param_output_feature_map_225 = 0*2;
  // Collect __xlx_output_feature_map_226__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_226__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_226__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_226)[i]);
}
  int __xlx_size_param_output_feature_map_226 = 480;
  int __xlx_offset_param_output_feature_map_226 = 0;
  int __xlx_offset_byte_param_output_feature_map_226 = 0*2;
  // Collect __xlx_output_feature_map_227__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_227__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_227__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_227)[i]);
}
  int __xlx_size_param_output_feature_map_227 = 480;
  int __xlx_offset_param_output_feature_map_227 = 0;
  int __xlx_offset_byte_param_output_feature_map_227 = 0*2;
  // Collect __xlx_output_feature_map_228__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_228__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_228__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_228)[i]);
}
  int __xlx_size_param_output_feature_map_228 = 480;
  int __xlx_offset_param_output_feature_map_228 = 0;
  int __xlx_offset_byte_param_output_feature_map_228 = 0*2;
  // Collect __xlx_output_feature_map_229__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_229__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_229__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_229)[i]);
}
  int __xlx_size_param_output_feature_map_229 = 480;
  int __xlx_offset_param_output_feature_map_229 = 0;
  int __xlx_offset_byte_param_output_feature_map_229 = 0*2;
  // Collect __xlx_output_feature_map_230__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_230__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_230__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_230)[i]);
}
  int __xlx_size_param_output_feature_map_230 = 480;
  int __xlx_offset_param_output_feature_map_230 = 0;
  int __xlx_offset_byte_param_output_feature_map_230 = 0*2;
  // Collect __xlx_output_feature_map_231__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_231__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_231__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_231)[i]);
}
  int __xlx_size_param_output_feature_map_231 = 480;
  int __xlx_offset_param_output_feature_map_231 = 0;
  int __xlx_offset_byte_param_output_feature_map_231 = 0*2;
  // Collect __xlx_output_feature_map_232__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_232__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_232__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_232)[i]);
}
  int __xlx_size_param_output_feature_map_232 = 480;
  int __xlx_offset_param_output_feature_map_232 = 0;
  int __xlx_offset_byte_param_output_feature_map_232 = 0*2;
  // Collect __xlx_output_feature_map_233__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_233__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_233__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_233)[i]);
}
  int __xlx_size_param_output_feature_map_233 = 480;
  int __xlx_offset_param_output_feature_map_233 = 0;
  int __xlx_offset_byte_param_output_feature_map_233 = 0*2;
  // Collect __xlx_output_feature_map_234__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_234__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_234__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_234)[i]);
}
  int __xlx_size_param_output_feature_map_234 = 480;
  int __xlx_offset_param_output_feature_map_234 = 0;
  int __xlx_offset_byte_param_output_feature_map_234 = 0*2;
  // Collect __xlx_output_feature_map_235__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_235__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_235__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_235)[i]);
}
  int __xlx_size_param_output_feature_map_235 = 480;
  int __xlx_offset_param_output_feature_map_235 = 0;
  int __xlx_offset_byte_param_output_feature_map_235 = 0*2;
  // Collect __xlx_output_feature_map_236__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_236__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_236__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_236)[i]);
}
  int __xlx_size_param_output_feature_map_236 = 480;
  int __xlx_offset_param_output_feature_map_236 = 0;
  int __xlx_offset_byte_param_output_feature_map_236 = 0*2;
  // Collect __xlx_output_feature_map_237__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_237__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_237__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_237)[i]);
}
  int __xlx_size_param_output_feature_map_237 = 480;
  int __xlx_offset_param_output_feature_map_237 = 0;
  int __xlx_offset_byte_param_output_feature_map_237 = 0*2;
  // Collect __xlx_output_feature_map_238__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_238__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_238__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_238)[i]);
}
  int __xlx_size_param_output_feature_map_238 = 480;
  int __xlx_offset_param_output_feature_map_238 = 0;
  int __xlx_offset_byte_param_output_feature_map_238 = 0*2;
  // Collect __xlx_output_feature_map_239__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_239__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_239__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_239)[i]);
}
  int __xlx_size_param_output_feature_map_239 = 480;
  int __xlx_offset_param_output_feature_map_239 = 0;
  int __xlx_offset_byte_param_output_feature_map_239 = 0*2;
  // Collect __xlx_output_feature_map_240__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_240__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_240__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_240)[i]);
}
  int __xlx_size_param_output_feature_map_240 = 480;
  int __xlx_offset_param_output_feature_map_240 = 0;
  int __xlx_offset_byte_param_output_feature_map_240 = 0*2;
  // Collect __xlx_output_feature_map_241__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_241__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_241__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_241)[i]);
}
  int __xlx_size_param_output_feature_map_241 = 480;
  int __xlx_offset_param_output_feature_map_241 = 0;
  int __xlx_offset_byte_param_output_feature_map_241 = 0*2;
  // Collect __xlx_output_feature_map_242__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_242__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_242__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_242)[i]);
}
  int __xlx_size_param_output_feature_map_242 = 480;
  int __xlx_offset_param_output_feature_map_242 = 0;
  int __xlx_offset_byte_param_output_feature_map_242 = 0*2;
  // Collect __xlx_output_feature_map_243__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_243__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_243__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_243)[i]);
}
  int __xlx_size_param_output_feature_map_243 = 480;
  int __xlx_offset_param_output_feature_map_243 = 0;
  int __xlx_offset_byte_param_output_feature_map_243 = 0*2;
  // Collect __xlx_output_feature_map_244__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_244__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_244__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_244)[i]);
}
  int __xlx_size_param_output_feature_map_244 = 480;
  int __xlx_offset_param_output_feature_map_244 = 0;
  int __xlx_offset_byte_param_output_feature_map_244 = 0*2;
  // Collect __xlx_output_feature_map_245__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_245__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_245__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_245)[i]);
}
  int __xlx_size_param_output_feature_map_245 = 480;
  int __xlx_offset_param_output_feature_map_245 = 0;
  int __xlx_offset_byte_param_output_feature_map_245 = 0*2;
  // Collect __xlx_output_feature_map_246__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_246__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_246__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_246)[i]);
}
  int __xlx_size_param_output_feature_map_246 = 480;
  int __xlx_offset_param_output_feature_map_246 = 0;
  int __xlx_offset_byte_param_output_feature_map_246 = 0*2;
  // Collect __xlx_output_feature_map_247__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_247__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_247__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_247)[i]);
}
  int __xlx_size_param_output_feature_map_247 = 480;
  int __xlx_offset_param_output_feature_map_247 = 0;
  int __xlx_offset_byte_param_output_feature_map_247 = 0*2;
  // Collect __xlx_output_feature_map_248__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_248__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_248__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_248)[i]);
}
  int __xlx_size_param_output_feature_map_248 = 480;
  int __xlx_offset_param_output_feature_map_248 = 0;
  int __xlx_offset_byte_param_output_feature_map_248 = 0*2;
  // Collect __xlx_output_feature_map_249__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_249__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_249__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_249)[i]);
}
  int __xlx_size_param_output_feature_map_249 = 480;
  int __xlx_offset_param_output_feature_map_249 = 0;
  int __xlx_offset_byte_param_output_feature_map_249 = 0*2;
  // Collect __xlx_output_feature_map_250__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_250__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_250__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_250)[i]);
}
  int __xlx_size_param_output_feature_map_250 = 480;
  int __xlx_offset_param_output_feature_map_250 = 0;
  int __xlx_offset_byte_param_output_feature_map_250 = 0*2;
  // Collect __xlx_output_feature_map_251__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_251__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_251__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_251)[i]);
}
  int __xlx_size_param_output_feature_map_251 = 480;
  int __xlx_offset_param_output_feature_map_251 = 0;
  int __xlx_offset_byte_param_output_feature_map_251 = 0*2;
  // Collect __xlx_output_feature_map_252__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_252__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_252__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_252)[i]);
}
  int __xlx_size_param_output_feature_map_252 = 480;
  int __xlx_offset_param_output_feature_map_252 = 0;
  int __xlx_offset_byte_param_output_feature_map_252 = 0*2;
  // Collect __xlx_output_feature_map_253__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_253__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_253__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_253)[i]);
}
  int __xlx_size_param_output_feature_map_253 = 480;
  int __xlx_offset_param_output_feature_map_253 = 0;
  int __xlx_offset_byte_param_output_feature_map_253 = 0*2;
  // Collect __xlx_output_feature_map_254__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_254__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_254__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_254)[i]);
}
  int __xlx_size_param_output_feature_map_254 = 480;
  int __xlx_offset_param_output_feature_map_254 = 0;
  int __xlx_offset_byte_param_output_feature_map_254 = 0*2;
  // Collect __xlx_output_feature_map_255__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_255__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_255__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_255)[i]);
}
  int __xlx_size_param_output_feature_map_255 = 480;
  int __xlx_offset_param_output_feature_map_255 = 0;
  int __xlx_offset_byte_param_output_feature_map_255 = 0*2;
  // Collect __xlx_output_feature_map_256__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_256__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_256__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_256)[i]);
}
  int __xlx_size_param_output_feature_map_256 = 480;
  int __xlx_offset_param_output_feature_map_256 = 0;
  int __xlx_offset_byte_param_output_feature_map_256 = 0*2;
  // Collect __xlx_output_feature_map_257__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_257__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_257__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_257)[i]);
}
  int __xlx_size_param_output_feature_map_257 = 480;
  int __xlx_offset_param_output_feature_map_257 = 0;
  int __xlx_offset_byte_param_output_feature_map_257 = 0*2;
  // Collect __xlx_output_feature_map_258__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_258__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_258__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_258)[i]);
}
  int __xlx_size_param_output_feature_map_258 = 480;
  int __xlx_offset_param_output_feature_map_258 = 0;
  int __xlx_offset_byte_param_output_feature_map_258 = 0*2;
  // Collect __xlx_output_feature_map_259__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_259__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_259__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_259)[i]);
}
  int __xlx_size_param_output_feature_map_259 = 480;
  int __xlx_offset_param_output_feature_map_259 = 0;
  int __xlx_offset_byte_param_output_feature_map_259 = 0*2;
  // Collect __xlx_output_feature_map_260__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_260__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_260__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_260)[i]);
}
  int __xlx_size_param_output_feature_map_260 = 480;
  int __xlx_offset_param_output_feature_map_260 = 0;
  int __xlx_offset_byte_param_output_feature_map_260 = 0*2;
  // Collect __xlx_output_feature_map_261__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_261__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_261__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_261)[i]);
}
  int __xlx_size_param_output_feature_map_261 = 480;
  int __xlx_offset_param_output_feature_map_261 = 0;
  int __xlx_offset_byte_param_output_feature_map_261 = 0*2;
  // Collect __xlx_output_feature_map_262__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_262__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_262__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_262)[i]);
}
  int __xlx_size_param_output_feature_map_262 = 480;
  int __xlx_offset_param_output_feature_map_262 = 0;
  int __xlx_offset_byte_param_output_feature_map_262 = 0*2;
  // Collect __xlx_output_feature_map_263__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_263__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_263__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_263)[i]);
}
  int __xlx_size_param_output_feature_map_263 = 480;
  int __xlx_offset_param_output_feature_map_263 = 0;
  int __xlx_offset_byte_param_output_feature_map_263 = 0*2;
  // Collect __xlx_output_feature_map_264__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_264__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_264__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_264)[i]);
}
  int __xlx_size_param_output_feature_map_264 = 480;
  int __xlx_offset_param_output_feature_map_264 = 0;
  int __xlx_offset_byte_param_output_feature_map_264 = 0*2;
  // Collect __xlx_output_feature_map_265__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_265__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_265__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_265)[i]);
}
  int __xlx_size_param_output_feature_map_265 = 480;
  int __xlx_offset_param_output_feature_map_265 = 0;
  int __xlx_offset_byte_param_output_feature_map_265 = 0*2;
  // Collect __xlx_output_feature_map_266__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_266__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_266__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_266)[i]);
}
  int __xlx_size_param_output_feature_map_266 = 480;
  int __xlx_offset_param_output_feature_map_266 = 0;
  int __xlx_offset_byte_param_output_feature_map_266 = 0*2;
  // Collect __xlx_output_feature_map_267__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_267__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_267__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_267)[i]);
}
  int __xlx_size_param_output_feature_map_267 = 480;
  int __xlx_offset_param_output_feature_map_267 = 0;
  int __xlx_offset_byte_param_output_feature_map_267 = 0*2;
  // Collect __xlx_output_feature_map_268__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_268__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_268__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_268)[i]);
}
  int __xlx_size_param_output_feature_map_268 = 480;
  int __xlx_offset_param_output_feature_map_268 = 0;
  int __xlx_offset_byte_param_output_feature_map_268 = 0*2;
  // Collect __xlx_output_feature_map_269__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_269__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_269__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_269)[i]);
}
  int __xlx_size_param_output_feature_map_269 = 480;
  int __xlx_offset_param_output_feature_map_269 = 0;
  int __xlx_offset_byte_param_output_feature_map_269 = 0*2;
  // Collect __xlx_output_feature_map_270__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_270__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_270__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_270)[i]);
}
  int __xlx_size_param_output_feature_map_270 = 480;
  int __xlx_offset_param_output_feature_map_270 = 0;
  int __xlx_offset_byte_param_output_feature_map_270 = 0*2;
  // Collect __xlx_output_feature_map_271__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_271__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_271__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_271)[i]);
}
  int __xlx_size_param_output_feature_map_271 = 480;
  int __xlx_offset_param_output_feature_map_271 = 0;
  int __xlx_offset_byte_param_output_feature_map_271 = 0*2;
  // Collect __xlx_output_feature_map_272__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_272__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_272__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_272)[i]);
}
  int __xlx_size_param_output_feature_map_272 = 480;
  int __xlx_offset_param_output_feature_map_272 = 0;
  int __xlx_offset_byte_param_output_feature_map_272 = 0*2;
  // Collect __xlx_output_feature_map_273__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_273__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_273__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_273)[i]);
}
  int __xlx_size_param_output_feature_map_273 = 480;
  int __xlx_offset_param_output_feature_map_273 = 0;
  int __xlx_offset_byte_param_output_feature_map_273 = 0*2;
  // Collect __xlx_output_feature_map_274__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_274__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_274__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_274)[i]);
}
  int __xlx_size_param_output_feature_map_274 = 480;
  int __xlx_offset_param_output_feature_map_274 = 0;
  int __xlx_offset_byte_param_output_feature_map_274 = 0*2;
  // Collect __xlx_output_feature_map_275__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_275__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_275__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_275)[i]);
}
  int __xlx_size_param_output_feature_map_275 = 480;
  int __xlx_offset_param_output_feature_map_275 = 0;
  int __xlx_offset_byte_param_output_feature_map_275 = 0*2;
  // Collect __xlx_output_feature_map_276__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_276__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_276__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_276)[i]);
}
  int __xlx_size_param_output_feature_map_276 = 480;
  int __xlx_offset_param_output_feature_map_276 = 0;
  int __xlx_offset_byte_param_output_feature_map_276 = 0*2;
  // Collect __xlx_output_feature_map_277__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_277__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_277__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_277)[i]);
}
  int __xlx_size_param_output_feature_map_277 = 480;
  int __xlx_offset_param_output_feature_map_277 = 0;
  int __xlx_offset_byte_param_output_feature_map_277 = 0*2;
  // Collect __xlx_output_feature_map_278__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_278__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_278__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_278)[i]);
}
  int __xlx_size_param_output_feature_map_278 = 480;
  int __xlx_offset_param_output_feature_map_278 = 0;
  int __xlx_offset_byte_param_output_feature_map_278 = 0*2;
  // Collect __xlx_output_feature_map_279__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_279__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_279__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_279)[i]);
}
  int __xlx_size_param_output_feature_map_279 = 480;
  int __xlx_offset_param_output_feature_map_279 = 0;
  int __xlx_offset_byte_param_output_feature_map_279 = 0*2;
  // Collect __xlx_output_feature_map_280__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_280__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_280__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_280)[i]);
}
  int __xlx_size_param_output_feature_map_280 = 480;
  int __xlx_offset_param_output_feature_map_280 = 0;
  int __xlx_offset_byte_param_output_feature_map_280 = 0*2;
  // Collect __xlx_output_feature_map_281__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_281__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_281__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_281)[i]);
}
  int __xlx_size_param_output_feature_map_281 = 480;
  int __xlx_offset_param_output_feature_map_281 = 0;
  int __xlx_offset_byte_param_output_feature_map_281 = 0*2;
  // Collect __xlx_output_feature_map_282__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_282__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_282__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_282)[i]);
}
  int __xlx_size_param_output_feature_map_282 = 480;
  int __xlx_offset_param_output_feature_map_282 = 0;
  int __xlx_offset_byte_param_output_feature_map_282 = 0*2;
  // Collect __xlx_output_feature_map_283__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_283__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_283__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_283)[i]);
}
  int __xlx_size_param_output_feature_map_283 = 480;
  int __xlx_offset_param_output_feature_map_283 = 0;
  int __xlx_offset_byte_param_output_feature_map_283 = 0*2;
  // Collect __xlx_output_feature_map_284__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_284__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_284__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_284)[i]);
}
  int __xlx_size_param_output_feature_map_284 = 480;
  int __xlx_offset_param_output_feature_map_284 = 0;
  int __xlx_offset_byte_param_output_feature_map_284 = 0*2;
  // Collect __xlx_output_feature_map_285__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_285__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_285__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_285)[i]);
}
  int __xlx_size_param_output_feature_map_285 = 480;
  int __xlx_offset_param_output_feature_map_285 = 0;
  int __xlx_offset_byte_param_output_feature_map_285 = 0*2;
  // Collect __xlx_output_feature_map_286__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_286__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_286__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_286)[i]);
}
  int __xlx_size_param_output_feature_map_286 = 480;
  int __xlx_offset_param_output_feature_map_286 = 0;
  int __xlx_offset_byte_param_output_feature_map_286 = 0*2;
  // Collect __xlx_output_feature_map_287__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_287__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_287__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_287)[i]);
}
  int __xlx_size_param_output_feature_map_287 = 480;
  int __xlx_offset_param_output_feature_map_287 = 0;
  int __xlx_offset_byte_param_output_feature_map_287 = 0*2;
  // Collect __xlx_output_feature_map_288__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_288__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_288__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_288)[i]);
}
  int __xlx_size_param_output_feature_map_288 = 480;
  int __xlx_offset_param_output_feature_map_288 = 0;
  int __xlx_offset_byte_param_output_feature_map_288 = 0*2;
  // Collect __xlx_output_feature_map_289__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_289__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_289__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_289)[i]);
}
  int __xlx_size_param_output_feature_map_289 = 480;
  int __xlx_offset_param_output_feature_map_289 = 0;
  int __xlx_offset_byte_param_output_feature_map_289 = 0*2;
  // Collect __xlx_output_feature_map_290__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_290__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_290__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_290)[i]);
}
  int __xlx_size_param_output_feature_map_290 = 480;
  int __xlx_offset_param_output_feature_map_290 = 0;
  int __xlx_offset_byte_param_output_feature_map_290 = 0*2;
  // Collect __xlx_output_feature_map_291__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_291__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_291__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_291)[i]);
}
  int __xlx_size_param_output_feature_map_291 = 480;
  int __xlx_offset_param_output_feature_map_291 = 0;
  int __xlx_offset_byte_param_output_feature_map_291 = 0*2;
  // Collect __xlx_output_feature_map_292__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_292__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_292__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_292)[i]);
}
  int __xlx_size_param_output_feature_map_292 = 480;
  int __xlx_offset_param_output_feature_map_292 = 0;
  int __xlx_offset_byte_param_output_feature_map_292 = 0*2;
  // Collect __xlx_output_feature_map_293__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_293__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_293__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_293)[i]);
}
  int __xlx_size_param_output_feature_map_293 = 480;
  int __xlx_offset_param_output_feature_map_293 = 0;
  int __xlx_offset_byte_param_output_feature_map_293 = 0*2;
  // Collect __xlx_output_feature_map_294__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_294__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_294__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_294)[i]);
}
  int __xlx_size_param_output_feature_map_294 = 480;
  int __xlx_offset_param_output_feature_map_294 = 0;
  int __xlx_offset_byte_param_output_feature_map_294 = 0*2;
  // Collect __xlx_output_feature_map_295__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_295__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_295__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_295)[i]);
}
  int __xlx_size_param_output_feature_map_295 = 480;
  int __xlx_offset_param_output_feature_map_295 = 0;
  int __xlx_offset_byte_param_output_feature_map_295 = 0*2;
  // Collect __xlx_output_feature_map_296__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_296__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_296__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_296)[i]);
}
  int __xlx_size_param_output_feature_map_296 = 480;
  int __xlx_offset_param_output_feature_map_296 = 0;
  int __xlx_offset_byte_param_output_feature_map_296 = 0*2;
  // Collect __xlx_output_feature_map_297__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_297__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_297__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_297)[i]);
}
  int __xlx_size_param_output_feature_map_297 = 480;
  int __xlx_offset_param_output_feature_map_297 = 0;
  int __xlx_offset_byte_param_output_feature_map_297 = 0*2;
  // Collect __xlx_output_feature_map_298__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_298__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_298__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_298)[i]);
}
  int __xlx_size_param_output_feature_map_298 = 480;
  int __xlx_offset_param_output_feature_map_298 = 0;
  int __xlx_offset_byte_param_output_feature_map_298 = 0*2;
  // Collect __xlx_output_feature_map_299__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_299__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_299__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_299)[i]);
}
  int __xlx_size_param_output_feature_map_299 = 480;
  int __xlx_offset_param_output_feature_map_299 = 0;
  int __xlx_offset_byte_param_output_feature_map_299 = 0*2;
  // Collect __xlx_output_feature_map_300__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_300__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_300__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_300)[i]);
}
  int __xlx_size_param_output_feature_map_300 = 480;
  int __xlx_offset_param_output_feature_map_300 = 0;
  int __xlx_offset_byte_param_output_feature_map_300 = 0*2;
  // Collect __xlx_output_feature_map_301__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_301__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_301__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_301)[i]);
}
  int __xlx_size_param_output_feature_map_301 = 480;
  int __xlx_offset_param_output_feature_map_301 = 0;
  int __xlx_offset_byte_param_output_feature_map_301 = 0*2;
  // Collect __xlx_output_feature_map_302__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_302__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_302__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_302)[i]);
}
  int __xlx_size_param_output_feature_map_302 = 480;
  int __xlx_offset_param_output_feature_map_302 = 0;
  int __xlx_offset_byte_param_output_feature_map_302 = 0*2;
  // Collect __xlx_output_feature_map_303__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_303__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_303__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_303)[i]);
}
  int __xlx_size_param_output_feature_map_303 = 480;
  int __xlx_offset_param_output_feature_map_303 = 0;
  int __xlx_offset_byte_param_output_feature_map_303 = 0*2;
  // Collect __xlx_output_feature_map_304__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_304__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_304__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_304)[i]);
}
  int __xlx_size_param_output_feature_map_304 = 480;
  int __xlx_offset_param_output_feature_map_304 = 0;
  int __xlx_offset_byte_param_output_feature_map_304 = 0*2;
  // Collect __xlx_output_feature_map_305__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_305__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_305__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_305)[i]);
}
  int __xlx_size_param_output_feature_map_305 = 480;
  int __xlx_offset_param_output_feature_map_305 = 0;
  int __xlx_offset_byte_param_output_feature_map_305 = 0*2;
  // Collect __xlx_output_feature_map_306__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_306__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_306__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_306)[i]);
}
  int __xlx_size_param_output_feature_map_306 = 480;
  int __xlx_offset_param_output_feature_map_306 = 0;
  int __xlx_offset_byte_param_output_feature_map_306 = 0*2;
  // Collect __xlx_output_feature_map_307__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_307__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_307__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_307)[i]);
}
  int __xlx_size_param_output_feature_map_307 = 480;
  int __xlx_offset_param_output_feature_map_307 = 0;
  int __xlx_offset_byte_param_output_feature_map_307 = 0*2;
  // Collect __xlx_output_feature_map_308__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_308__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_308__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_308)[i]);
}
  int __xlx_size_param_output_feature_map_308 = 480;
  int __xlx_offset_param_output_feature_map_308 = 0;
  int __xlx_offset_byte_param_output_feature_map_308 = 0*2;
  // Collect __xlx_output_feature_map_309__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_309__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_309__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_309)[i]);
}
  int __xlx_size_param_output_feature_map_309 = 480;
  int __xlx_offset_param_output_feature_map_309 = 0;
  int __xlx_offset_byte_param_output_feature_map_309 = 0*2;
  // Collect __xlx_output_feature_map_310__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_310__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_310__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_310)[i]);
}
  int __xlx_size_param_output_feature_map_310 = 480;
  int __xlx_offset_param_output_feature_map_310 = 0;
  int __xlx_offset_byte_param_output_feature_map_310 = 0*2;
  // Collect __xlx_output_feature_map_311__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_311__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_311__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_311)[i]);
}
  int __xlx_size_param_output_feature_map_311 = 480;
  int __xlx_offset_param_output_feature_map_311 = 0;
  int __xlx_offset_byte_param_output_feature_map_311 = 0*2;
  // Collect __xlx_output_feature_map_312__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_312__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_312__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_312)[i]);
}
  int __xlx_size_param_output_feature_map_312 = 480;
  int __xlx_offset_param_output_feature_map_312 = 0;
  int __xlx_offset_byte_param_output_feature_map_312 = 0*2;
  // Collect __xlx_output_feature_map_313__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_313__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_313__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_313)[i]);
}
  int __xlx_size_param_output_feature_map_313 = 480;
  int __xlx_offset_param_output_feature_map_313 = 0;
  int __xlx_offset_byte_param_output_feature_map_313 = 0*2;
  // Collect __xlx_output_feature_map_314__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_314__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_314__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_314)[i]);
}
  int __xlx_size_param_output_feature_map_314 = 480;
  int __xlx_offset_param_output_feature_map_314 = 0;
  int __xlx_offset_byte_param_output_feature_map_314 = 0*2;
  // Collect __xlx_output_feature_map_315__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_315__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_315__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_315)[i]);
}
  int __xlx_size_param_output_feature_map_315 = 480;
  int __xlx_offset_param_output_feature_map_315 = 0;
  int __xlx_offset_byte_param_output_feature_map_315 = 0*2;
  // Collect __xlx_output_feature_map_316__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_316__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_316__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_316)[i]);
}
  int __xlx_size_param_output_feature_map_316 = 480;
  int __xlx_offset_param_output_feature_map_316 = 0;
  int __xlx_offset_byte_param_output_feature_map_316 = 0*2;
  // Collect __xlx_output_feature_map_317__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_317__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_317__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_317)[i]);
}
  int __xlx_size_param_output_feature_map_317 = 480;
  int __xlx_offset_param_output_feature_map_317 = 0;
  int __xlx_offset_byte_param_output_feature_map_317 = 0*2;
  // Collect __xlx_output_feature_map_318__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_318__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_318__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_318)[i]);
}
  int __xlx_size_param_output_feature_map_318 = 480;
  int __xlx_offset_param_output_feature_map_318 = 0;
  int __xlx_offset_byte_param_output_feature_map_318 = 0*2;
  // Collect __xlx_output_feature_map_319__tmp_vec
std::vector<Byte<2>> __xlx_output_feature_map_319__tmp_vec;
for (size_t i = 0; i < 480; ++i){
__xlx_output_feature_map_319__tmp_vec.push_back(((Byte<2>*)__xlx_apatb_param_output_feature_map_319)[i]);
}
  int __xlx_size_param_output_feature_map_319 = 480;
  int __xlx_offset_param_output_feature_map_319 = 0;
  int __xlx_offset_byte_param_output_feature_map_319 = 0*2;
  // DUT call
  main_func(__xlx_input_feature_map__tmp_vec.data(), __xlx_layer_weights_layer_bias__tmp_vec.data(), __xlx_output_feature_map_0__tmp_vec.data(), __xlx_output_feature_map_1__tmp_vec.data(), __xlx_output_feature_map_2__tmp_vec.data(), __xlx_output_feature_map_3__tmp_vec.data(), __xlx_output_feature_map_4__tmp_vec.data(), __xlx_output_feature_map_5__tmp_vec.data(), __xlx_output_feature_map_6__tmp_vec.data(), __xlx_output_feature_map_7__tmp_vec.data(), __xlx_output_feature_map_8__tmp_vec.data(), __xlx_output_feature_map_9__tmp_vec.data(), __xlx_output_feature_map_10__tmp_vec.data(), __xlx_output_feature_map_11__tmp_vec.data(), __xlx_output_feature_map_12__tmp_vec.data(), __xlx_output_feature_map_13__tmp_vec.data(), __xlx_output_feature_map_14__tmp_vec.data(), __xlx_output_feature_map_15__tmp_vec.data(), __xlx_output_feature_map_16__tmp_vec.data(), __xlx_output_feature_map_17__tmp_vec.data(), __xlx_output_feature_map_18__tmp_vec.data(), __xlx_output_feature_map_19__tmp_vec.data(), __xlx_output_feature_map_20__tmp_vec.data(), __xlx_output_feature_map_21__tmp_vec.data(), __xlx_output_feature_map_22__tmp_vec.data(), __xlx_output_feature_map_23__tmp_vec.data(), __xlx_output_feature_map_24__tmp_vec.data(), __xlx_output_feature_map_25__tmp_vec.data(), __xlx_output_feature_map_26__tmp_vec.data(), __xlx_output_feature_map_27__tmp_vec.data(), __xlx_output_feature_map_28__tmp_vec.data(), __xlx_output_feature_map_29__tmp_vec.data(), __xlx_output_feature_map_30__tmp_vec.data(), __xlx_output_feature_map_31__tmp_vec.data(), __xlx_output_feature_map_32__tmp_vec.data(), __xlx_output_feature_map_33__tmp_vec.data(), __xlx_output_feature_map_34__tmp_vec.data(), __xlx_output_feature_map_35__tmp_vec.data(), __xlx_output_feature_map_36__tmp_vec.data(), __xlx_output_feature_map_37__tmp_vec.data(), __xlx_output_feature_map_38__tmp_vec.data(), __xlx_output_feature_map_39__tmp_vec.data(), __xlx_output_feature_map_40__tmp_vec.data(), __xlx_output_feature_map_41__tmp_vec.data(), __xlx_output_feature_map_42__tmp_vec.data(), __xlx_output_feature_map_43__tmp_vec.data(), __xlx_output_feature_map_44__tmp_vec.data(), __xlx_output_feature_map_45__tmp_vec.data(), __xlx_output_feature_map_46__tmp_vec.data(), __xlx_output_feature_map_47__tmp_vec.data(), __xlx_output_feature_map_48__tmp_vec.data(), __xlx_output_feature_map_49__tmp_vec.data(), __xlx_output_feature_map_50__tmp_vec.data(), __xlx_output_feature_map_51__tmp_vec.data(), __xlx_output_feature_map_52__tmp_vec.data(), __xlx_output_feature_map_53__tmp_vec.data(), __xlx_output_feature_map_54__tmp_vec.data(), __xlx_output_feature_map_55__tmp_vec.data(), __xlx_output_feature_map_56__tmp_vec.data(), __xlx_output_feature_map_57__tmp_vec.data(), __xlx_output_feature_map_58__tmp_vec.data(), __xlx_output_feature_map_59__tmp_vec.data(), __xlx_output_feature_map_60__tmp_vec.data(), __xlx_output_feature_map_61__tmp_vec.data(), __xlx_output_feature_map_62__tmp_vec.data(), __xlx_output_feature_map_63__tmp_vec.data(), __xlx_output_feature_map_64__tmp_vec.data(), __xlx_output_feature_map_65__tmp_vec.data(), __xlx_output_feature_map_66__tmp_vec.data(), __xlx_output_feature_map_67__tmp_vec.data(), __xlx_output_feature_map_68__tmp_vec.data(), __xlx_output_feature_map_69__tmp_vec.data(), __xlx_output_feature_map_70__tmp_vec.data(), __xlx_output_feature_map_71__tmp_vec.data(), __xlx_output_feature_map_72__tmp_vec.data(), __xlx_output_feature_map_73__tmp_vec.data(), __xlx_output_feature_map_74__tmp_vec.data(), __xlx_output_feature_map_75__tmp_vec.data(), __xlx_output_feature_map_76__tmp_vec.data(), __xlx_output_feature_map_77__tmp_vec.data(), __xlx_output_feature_map_78__tmp_vec.data(), __xlx_output_feature_map_79__tmp_vec.data(), __xlx_output_feature_map_80__tmp_vec.data(), __xlx_output_feature_map_81__tmp_vec.data(), __xlx_output_feature_map_82__tmp_vec.data(), __xlx_output_feature_map_83__tmp_vec.data(), __xlx_output_feature_map_84__tmp_vec.data(), __xlx_output_feature_map_85__tmp_vec.data(), __xlx_output_feature_map_86__tmp_vec.data(), __xlx_output_feature_map_87__tmp_vec.data(), __xlx_output_feature_map_88__tmp_vec.data(), __xlx_output_feature_map_89__tmp_vec.data(), __xlx_output_feature_map_90__tmp_vec.data(), __xlx_output_feature_map_91__tmp_vec.data(), __xlx_output_feature_map_92__tmp_vec.data(), __xlx_output_feature_map_93__tmp_vec.data(), __xlx_output_feature_map_94__tmp_vec.data(), __xlx_output_feature_map_95__tmp_vec.data(), __xlx_output_feature_map_96__tmp_vec.data(), __xlx_output_feature_map_97__tmp_vec.data(), __xlx_output_feature_map_98__tmp_vec.data(), __xlx_output_feature_map_99__tmp_vec.data(), __xlx_output_feature_map_100__tmp_vec.data(), __xlx_output_feature_map_101__tmp_vec.data(), __xlx_output_feature_map_102__tmp_vec.data(), __xlx_output_feature_map_103__tmp_vec.data(), __xlx_output_feature_map_104__tmp_vec.data(), __xlx_output_feature_map_105__tmp_vec.data(), __xlx_output_feature_map_106__tmp_vec.data(), __xlx_output_feature_map_107__tmp_vec.data(), __xlx_output_feature_map_108__tmp_vec.data(), __xlx_output_feature_map_109__tmp_vec.data(), __xlx_output_feature_map_110__tmp_vec.data(), __xlx_output_feature_map_111__tmp_vec.data(), __xlx_output_feature_map_112__tmp_vec.data(), __xlx_output_feature_map_113__tmp_vec.data(), __xlx_output_feature_map_114__tmp_vec.data(), __xlx_output_feature_map_115__tmp_vec.data(), __xlx_output_feature_map_116__tmp_vec.data(), __xlx_output_feature_map_117__tmp_vec.data(), __xlx_output_feature_map_118__tmp_vec.data(), __xlx_output_feature_map_119__tmp_vec.data(), __xlx_output_feature_map_120__tmp_vec.data(), __xlx_output_feature_map_121__tmp_vec.data(), __xlx_output_feature_map_122__tmp_vec.data(), __xlx_output_feature_map_123__tmp_vec.data(), __xlx_output_feature_map_124__tmp_vec.data(), __xlx_output_feature_map_125__tmp_vec.data(), __xlx_output_feature_map_126__tmp_vec.data(), __xlx_output_feature_map_127__tmp_vec.data(), __xlx_output_feature_map_128__tmp_vec.data(), __xlx_output_feature_map_129__tmp_vec.data(), __xlx_output_feature_map_130__tmp_vec.data(), __xlx_output_feature_map_131__tmp_vec.data(), __xlx_output_feature_map_132__tmp_vec.data(), __xlx_output_feature_map_133__tmp_vec.data(), __xlx_output_feature_map_134__tmp_vec.data(), __xlx_output_feature_map_135__tmp_vec.data(), __xlx_output_feature_map_136__tmp_vec.data(), __xlx_output_feature_map_137__tmp_vec.data(), __xlx_output_feature_map_138__tmp_vec.data(), __xlx_output_feature_map_139__tmp_vec.data(), __xlx_output_feature_map_140__tmp_vec.data(), __xlx_output_feature_map_141__tmp_vec.data(), __xlx_output_feature_map_142__tmp_vec.data(), __xlx_output_feature_map_143__tmp_vec.data(), __xlx_output_feature_map_144__tmp_vec.data(), __xlx_output_feature_map_145__tmp_vec.data(), __xlx_output_feature_map_146__tmp_vec.data(), __xlx_output_feature_map_147__tmp_vec.data(), __xlx_output_feature_map_148__tmp_vec.data(), __xlx_output_feature_map_149__tmp_vec.data(), __xlx_output_feature_map_150__tmp_vec.data(), __xlx_output_feature_map_151__tmp_vec.data(), __xlx_output_feature_map_152__tmp_vec.data(), __xlx_output_feature_map_153__tmp_vec.data(), __xlx_output_feature_map_154__tmp_vec.data(), __xlx_output_feature_map_155__tmp_vec.data(), __xlx_output_feature_map_156__tmp_vec.data(), __xlx_output_feature_map_157__tmp_vec.data(), __xlx_output_feature_map_158__tmp_vec.data(), __xlx_output_feature_map_159__tmp_vec.data(), __xlx_output_feature_map_160__tmp_vec.data(), __xlx_output_feature_map_161__tmp_vec.data(), __xlx_output_feature_map_162__tmp_vec.data(), __xlx_output_feature_map_163__tmp_vec.data(), __xlx_output_feature_map_164__tmp_vec.data(), __xlx_output_feature_map_165__tmp_vec.data(), __xlx_output_feature_map_166__tmp_vec.data(), __xlx_output_feature_map_167__tmp_vec.data(), __xlx_output_feature_map_168__tmp_vec.data(), __xlx_output_feature_map_169__tmp_vec.data(), __xlx_output_feature_map_170__tmp_vec.data(), __xlx_output_feature_map_171__tmp_vec.data(), __xlx_output_feature_map_172__tmp_vec.data(), __xlx_output_feature_map_173__tmp_vec.data(), __xlx_output_feature_map_174__tmp_vec.data(), __xlx_output_feature_map_175__tmp_vec.data(), __xlx_output_feature_map_176__tmp_vec.data(), __xlx_output_feature_map_177__tmp_vec.data(), __xlx_output_feature_map_178__tmp_vec.data(), __xlx_output_feature_map_179__tmp_vec.data(), __xlx_output_feature_map_180__tmp_vec.data(), __xlx_output_feature_map_181__tmp_vec.data(), __xlx_output_feature_map_182__tmp_vec.data(), __xlx_output_feature_map_183__tmp_vec.data(), __xlx_output_feature_map_184__tmp_vec.data(), __xlx_output_feature_map_185__tmp_vec.data(), __xlx_output_feature_map_186__tmp_vec.data(), __xlx_output_feature_map_187__tmp_vec.data(), __xlx_output_feature_map_188__tmp_vec.data(), __xlx_output_feature_map_189__tmp_vec.data(), __xlx_output_feature_map_190__tmp_vec.data(), __xlx_output_feature_map_191__tmp_vec.data(), __xlx_output_feature_map_192__tmp_vec.data(), __xlx_output_feature_map_193__tmp_vec.data(), __xlx_output_feature_map_194__tmp_vec.data(), __xlx_output_feature_map_195__tmp_vec.data(), __xlx_output_feature_map_196__tmp_vec.data(), __xlx_output_feature_map_197__tmp_vec.data(), __xlx_output_feature_map_198__tmp_vec.data(), __xlx_output_feature_map_199__tmp_vec.data(), __xlx_output_feature_map_200__tmp_vec.data(), __xlx_output_feature_map_201__tmp_vec.data(), __xlx_output_feature_map_202__tmp_vec.data(), __xlx_output_feature_map_203__tmp_vec.data(), __xlx_output_feature_map_204__tmp_vec.data(), __xlx_output_feature_map_205__tmp_vec.data(), __xlx_output_feature_map_206__tmp_vec.data(), __xlx_output_feature_map_207__tmp_vec.data(), __xlx_output_feature_map_208__tmp_vec.data(), __xlx_output_feature_map_209__tmp_vec.data(), __xlx_output_feature_map_210__tmp_vec.data(), __xlx_output_feature_map_211__tmp_vec.data(), __xlx_output_feature_map_212__tmp_vec.data(), __xlx_output_feature_map_213__tmp_vec.data(), __xlx_output_feature_map_214__tmp_vec.data(), __xlx_output_feature_map_215__tmp_vec.data(), __xlx_output_feature_map_216__tmp_vec.data(), __xlx_output_feature_map_217__tmp_vec.data(), __xlx_output_feature_map_218__tmp_vec.data(), __xlx_output_feature_map_219__tmp_vec.data(), __xlx_output_feature_map_220__tmp_vec.data(), __xlx_output_feature_map_221__tmp_vec.data(), __xlx_output_feature_map_222__tmp_vec.data(), __xlx_output_feature_map_223__tmp_vec.data(), __xlx_output_feature_map_224__tmp_vec.data(), __xlx_output_feature_map_225__tmp_vec.data(), __xlx_output_feature_map_226__tmp_vec.data(), __xlx_output_feature_map_227__tmp_vec.data(), __xlx_output_feature_map_228__tmp_vec.data(), __xlx_output_feature_map_229__tmp_vec.data(), __xlx_output_feature_map_230__tmp_vec.data(), __xlx_output_feature_map_231__tmp_vec.data(), __xlx_output_feature_map_232__tmp_vec.data(), __xlx_output_feature_map_233__tmp_vec.data(), __xlx_output_feature_map_234__tmp_vec.data(), __xlx_output_feature_map_235__tmp_vec.data(), __xlx_output_feature_map_236__tmp_vec.data(), __xlx_output_feature_map_237__tmp_vec.data(), __xlx_output_feature_map_238__tmp_vec.data(), __xlx_output_feature_map_239__tmp_vec.data(), __xlx_output_feature_map_240__tmp_vec.data(), __xlx_output_feature_map_241__tmp_vec.data(), __xlx_output_feature_map_242__tmp_vec.data(), __xlx_output_feature_map_243__tmp_vec.data(), __xlx_output_feature_map_244__tmp_vec.data(), __xlx_output_feature_map_245__tmp_vec.data(), __xlx_output_feature_map_246__tmp_vec.data(), __xlx_output_feature_map_247__tmp_vec.data(), __xlx_output_feature_map_248__tmp_vec.data(), __xlx_output_feature_map_249__tmp_vec.data(), __xlx_output_feature_map_250__tmp_vec.data(), __xlx_output_feature_map_251__tmp_vec.data(), __xlx_output_feature_map_252__tmp_vec.data(), __xlx_output_feature_map_253__tmp_vec.data(), __xlx_output_feature_map_254__tmp_vec.data(), __xlx_output_feature_map_255__tmp_vec.data(), __xlx_output_feature_map_256__tmp_vec.data(), __xlx_output_feature_map_257__tmp_vec.data(), __xlx_output_feature_map_258__tmp_vec.data(), __xlx_output_feature_map_259__tmp_vec.data(), __xlx_output_feature_map_260__tmp_vec.data(), __xlx_output_feature_map_261__tmp_vec.data(), __xlx_output_feature_map_262__tmp_vec.data(), __xlx_output_feature_map_263__tmp_vec.data(), __xlx_output_feature_map_264__tmp_vec.data(), __xlx_output_feature_map_265__tmp_vec.data(), __xlx_output_feature_map_266__tmp_vec.data(), __xlx_output_feature_map_267__tmp_vec.data(), __xlx_output_feature_map_268__tmp_vec.data(), __xlx_output_feature_map_269__tmp_vec.data(), __xlx_output_feature_map_270__tmp_vec.data(), __xlx_output_feature_map_271__tmp_vec.data(), __xlx_output_feature_map_272__tmp_vec.data(), __xlx_output_feature_map_273__tmp_vec.data(), __xlx_output_feature_map_274__tmp_vec.data(), __xlx_output_feature_map_275__tmp_vec.data(), __xlx_output_feature_map_276__tmp_vec.data(), __xlx_output_feature_map_277__tmp_vec.data(), __xlx_output_feature_map_278__tmp_vec.data(), __xlx_output_feature_map_279__tmp_vec.data(), __xlx_output_feature_map_280__tmp_vec.data(), __xlx_output_feature_map_281__tmp_vec.data(), __xlx_output_feature_map_282__tmp_vec.data(), __xlx_output_feature_map_283__tmp_vec.data(), __xlx_output_feature_map_284__tmp_vec.data(), __xlx_output_feature_map_285__tmp_vec.data(), __xlx_output_feature_map_286__tmp_vec.data(), __xlx_output_feature_map_287__tmp_vec.data(), __xlx_output_feature_map_288__tmp_vec.data(), __xlx_output_feature_map_289__tmp_vec.data(), __xlx_output_feature_map_290__tmp_vec.data(), __xlx_output_feature_map_291__tmp_vec.data(), __xlx_output_feature_map_292__tmp_vec.data(), __xlx_output_feature_map_293__tmp_vec.data(), __xlx_output_feature_map_294__tmp_vec.data(), __xlx_output_feature_map_295__tmp_vec.data(), __xlx_output_feature_map_296__tmp_vec.data(), __xlx_output_feature_map_297__tmp_vec.data(), __xlx_output_feature_map_298__tmp_vec.data(), __xlx_output_feature_map_299__tmp_vec.data(), __xlx_output_feature_map_300__tmp_vec.data(), __xlx_output_feature_map_301__tmp_vec.data(), __xlx_output_feature_map_302__tmp_vec.data(), __xlx_output_feature_map_303__tmp_vec.data(), __xlx_output_feature_map_304__tmp_vec.data(), __xlx_output_feature_map_305__tmp_vec.data(), __xlx_output_feature_map_306__tmp_vec.data(), __xlx_output_feature_map_307__tmp_vec.data(), __xlx_output_feature_map_308__tmp_vec.data(), __xlx_output_feature_map_309__tmp_vec.data(), __xlx_output_feature_map_310__tmp_vec.data(), __xlx_output_feature_map_311__tmp_vec.data(), __xlx_output_feature_map_312__tmp_vec.data(), __xlx_output_feature_map_313__tmp_vec.data(), __xlx_output_feature_map_314__tmp_vec.data(), __xlx_output_feature_map_315__tmp_vec.data(), __xlx_output_feature_map_316__tmp_vec.data(), __xlx_output_feature_map_317__tmp_vec.data(), __xlx_output_feature_map_318__tmp_vec.data(), __xlx_output_feature_map_319__tmp_vec.data(), __xlx_offset_byte_param_input_feature_map, __xlx_offset_byte_param_layer_weights, __xlx_offset_byte_param_layer_bias, __xlx_offset_byte_param_output_feature_map_0, __xlx_offset_byte_param_output_feature_map_1, __xlx_offset_byte_param_output_feature_map_2, __xlx_offset_byte_param_output_feature_map_3, __xlx_offset_byte_param_output_feature_map_4, __xlx_offset_byte_param_output_feature_map_5, __xlx_offset_byte_param_output_feature_map_6, __xlx_offset_byte_param_output_feature_map_7, __xlx_offset_byte_param_output_feature_map_8, __xlx_offset_byte_param_output_feature_map_9, __xlx_offset_byte_param_output_feature_map_10, __xlx_offset_byte_param_output_feature_map_11, __xlx_offset_byte_param_output_feature_map_12, __xlx_offset_byte_param_output_feature_map_13, __xlx_offset_byte_param_output_feature_map_14, __xlx_offset_byte_param_output_feature_map_15, __xlx_offset_byte_param_output_feature_map_16, __xlx_offset_byte_param_output_feature_map_17, __xlx_offset_byte_param_output_feature_map_18, __xlx_offset_byte_param_output_feature_map_19, __xlx_offset_byte_param_output_feature_map_20, __xlx_offset_byte_param_output_feature_map_21, __xlx_offset_byte_param_output_feature_map_22, __xlx_offset_byte_param_output_feature_map_23, __xlx_offset_byte_param_output_feature_map_24, __xlx_offset_byte_param_output_feature_map_25, __xlx_offset_byte_param_output_feature_map_26, __xlx_offset_byte_param_output_feature_map_27, __xlx_offset_byte_param_output_feature_map_28, __xlx_offset_byte_param_output_feature_map_29, __xlx_offset_byte_param_output_feature_map_30, __xlx_offset_byte_param_output_feature_map_31, __xlx_offset_byte_param_output_feature_map_32, __xlx_offset_byte_param_output_feature_map_33, __xlx_offset_byte_param_output_feature_map_34, __xlx_offset_byte_param_output_feature_map_35, __xlx_offset_byte_param_output_feature_map_36, __xlx_offset_byte_param_output_feature_map_37, __xlx_offset_byte_param_output_feature_map_38, __xlx_offset_byte_param_output_feature_map_39, __xlx_offset_byte_param_output_feature_map_40, __xlx_offset_byte_param_output_feature_map_41, __xlx_offset_byte_param_output_feature_map_42, __xlx_offset_byte_param_output_feature_map_43, __xlx_offset_byte_param_output_feature_map_44, __xlx_offset_byte_param_output_feature_map_45, __xlx_offset_byte_param_output_feature_map_46, __xlx_offset_byte_param_output_feature_map_47, __xlx_offset_byte_param_output_feature_map_48, __xlx_offset_byte_param_output_feature_map_49, __xlx_offset_byte_param_output_feature_map_50, __xlx_offset_byte_param_output_feature_map_51, __xlx_offset_byte_param_output_feature_map_52, __xlx_offset_byte_param_output_feature_map_53, __xlx_offset_byte_param_output_feature_map_54, __xlx_offset_byte_param_output_feature_map_55, __xlx_offset_byte_param_output_feature_map_56, __xlx_offset_byte_param_output_feature_map_57, __xlx_offset_byte_param_output_feature_map_58, __xlx_offset_byte_param_output_feature_map_59, __xlx_offset_byte_param_output_feature_map_60, __xlx_offset_byte_param_output_feature_map_61, __xlx_offset_byte_param_output_feature_map_62, __xlx_offset_byte_param_output_feature_map_63, __xlx_offset_byte_param_output_feature_map_64, __xlx_offset_byte_param_output_feature_map_65, __xlx_offset_byte_param_output_feature_map_66, __xlx_offset_byte_param_output_feature_map_67, __xlx_offset_byte_param_output_feature_map_68, __xlx_offset_byte_param_output_feature_map_69, __xlx_offset_byte_param_output_feature_map_70, __xlx_offset_byte_param_output_feature_map_71, __xlx_offset_byte_param_output_feature_map_72, __xlx_offset_byte_param_output_feature_map_73, __xlx_offset_byte_param_output_feature_map_74, __xlx_offset_byte_param_output_feature_map_75, __xlx_offset_byte_param_output_feature_map_76, __xlx_offset_byte_param_output_feature_map_77, __xlx_offset_byte_param_output_feature_map_78, __xlx_offset_byte_param_output_feature_map_79, __xlx_offset_byte_param_output_feature_map_80, __xlx_offset_byte_param_output_feature_map_81, __xlx_offset_byte_param_output_feature_map_82, __xlx_offset_byte_param_output_feature_map_83, __xlx_offset_byte_param_output_feature_map_84, __xlx_offset_byte_param_output_feature_map_85, __xlx_offset_byte_param_output_feature_map_86, __xlx_offset_byte_param_output_feature_map_87, __xlx_offset_byte_param_output_feature_map_88, __xlx_offset_byte_param_output_feature_map_89, __xlx_offset_byte_param_output_feature_map_90, __xlx_offset_byte_param_output_feature_map_91, __xlx_offset_byte_param_output_feature_map_92, __xlx_offset_byte_param_output_feature_map_93, __xlx_offset_byte_param_output_feature_map_94, __xlx_offset_byte_param_output_feature_map_95, __xlx_offset_byte_param_output_feature_map_96, __xlx_offset_byte_param_output_feature_map_97, __xlx_offset_byte_param_output_feature_map_98, __xlx_offset_byte_param_output_feature_map_99, __xlx_offset_byte_param_output_feature_map_100, __xlx_offset_byte_param_output_feature_map_101, __xlx_offset_byte_param_output_feature_map_102, __xlx_offset_byte_param_output_feature_map_103, __xlx_offset_byte_param_output_feature_map_104, __xlx_offset_byte_param_output_feature_map_105, __xlx_offset_byte_param_output_feature_map_106, __xlx_offset_byte_param_output_feature_map_107, __xlx_offset_byte_param_output_feature_map_108, __xlx_offset_byte_param_output_feature_map_109, __xlx_offset_byte_param_output_feature_map_110, __xlx_offset_byte_param_output_feature_map_111, __xlx_offset_byte_param_output_feature_map_112, __xlx_offset_byte_param_output_feature_map_113, __xlx_offset_byte_param_output_feature_map_114, __xlx_offset_byte_param_output_feature_map_115, __xlx_offset_byte_param_output_feature_map_116, __xlx_offset_byte_param_output_feature_map_117, __xlx_offset_byte_param_output_feature_map_118, __xlx_offset_byte_param_output_feature_map_119, __xlx_offset_byte_param_output_feature_map_120, __xlx_offset_byte_param_output_feature_map_121, __xlx_offset_byte_param_output_feature_map_122, __xlx_offset_byte_param_output_feature_map_123, __xlx_offset_byte_param_output_feature_map_124, __xlx_offset_byte_param_output_feature_map_125, __xlx_offset_byte_param_output_feature_map_126, __xlx_offset_byte_param_output_feature_map_127, __xlx_offset_byte_param_output_feature_map_128, __xlx_offset_byte_param_output_feature_map_129, __xlx_offset_byte_param_output_feature_map_130, __xlx_offset_byte_param_output_feature_map_131, __xlx_offset_byte_param_output_feature_map_132, __xlx_offset_byte_param_output_feature_map_133, __xlx_offset_byte_param_output_feature_map_134, __xlx_offset_byte_param_output_feature_map_135, __xlx_offset_byte_param_output_feature_map_136, __xlx_offset_byte_param_output_feature_map_137, __xlx_offset_byte_param_output_feature_map_138, __xlx_offset_byte_param_output_feature_map_139, __xlx_offset_byte_param_output_feature_map_140, __xlx_offset_byte_param_output_feature_map_141, __xlx_offset_byte_param_output_feature_map_142, __xlx_offset_byte_param_output_feature_map_143, __xlx_offset_byte_param_output_feature_map_144, __xlx_offset_byte_param_output_feature_map_145, __xlx_offset_byte_param_output_feature_map_146, __xlx_offset_byte_param_output_feature_map_147, __xlx_offset_byte_param_output_feature_map_148, __xlx_offset_byte_param_output_feature_map_149, __xlx_offset_byte_param_output_feature_map_150, __xlx_offset_byte_param_output_feature_map_151, __xlx_offset_byte_param_output_feature_map_152, __xlx_offset_byte_param_output_feature_map_153, __xlx_offset_byte_param_output_feature_map_154, __xlx_offset_byte_param_output_feature_map_155, __xlx_offset_byte_param_output_feature_map_156, __xlx_offset_byte_param_output_feature_map_157, __xlx_offset_byte_param_output_feature_map_158, __xlx_offset_byte_param_output_feature_map_159, __xlx_offset_byte_param_output_feature_map_160, __xlx_offset_byte_param_output_feature_map_161, __xlx_offset_byte_param_output_feature_map_162, __xlx_offset_byte_param_output_feature_map_163, __xlx_offset_byte_param_output_feature_map_164, __xlx_offset_byte_param_output_feature_map_165, __xlx_offset_byte_param_output_feature_map_166, __xlx_offset_byte_param_output_feature_map_167, __xlx_offset_byte_param_output_feature_map_168, __xlx_offset_byte_param_output_feature_map_169, __xlx_offset_byte_param_output_feature_map_170, __xlx_offset_byte_param_output_feature_map_171, __xlx_offset_byte_param_output_feature_map_172, __xlx_offset_byte_param_output_feature_map_173, __xlx_offset_byte_param_output_feature_map_174, __xlx_offset_byte_param_output_feature_map_175, __xlx_offset_byte_param_output_feature_map_176, __xlx_offset_byte_param_output_feature_map_177, __xlx_offset_byte_param_output_feature_map_178, __xlx_offset_byte_param_output_feature_map_179, __xlx_offset_byte_param_output_feature_map_180, __xlx_offset_byte_param_output_feature_map_181, __xlx_offset_byte_param_output_feature_map_182, __xlx_offset_byte_param_output_feature_map_183, __xlx_offset_byte_param_output_feature_map_184, __xlx_offset_byte_param_output_feature_map_185, __xlx_offset_byte_param_output_feature_map_186, __xlx_offset_byte_param_output_feature_map_187, __xlx_offset_byte_param_output_feature_map_188, __xlx_offset_byte_param_output_feature_map_189, __xlx_offset_byte_param_output_feature_map_190, __xlx_offset_byte_param_output_feature_map_191, __xlx_offset_byte_param_output_feature_map_192, __xlx_offset_byte_param_output_feature_map_193, __xlx_offset_byte_param_output_feature_map_194, __xlx_offset_byte_param_output_feature_map_195, __xlx_offset_byte_param_output_feature_map_196, __xlx_offset_byte_param_output_feature_map_197, __xlx_offset_byte_param_output_feature_map_198, __xlx_offset_byte_param_output_feature_map_199, __xlx_offset_byte_param_output_feature_map_200, __xlx_offset_byte_param_output_feature_map_201, __xlx_offset_byte_param_output_feature_map_202, __xlx_offset_byte_param_output_feature_map_203, __xlx_offset_byte_param_output_feature_map_204, __xlx_offset_byte_param_output_feature_map_205, __xlx_offset_byte_param_output_feature_map_206, __xlx_offset_byte_param_output_feature_map_207, __xlx_offset_byte_param_output_feature_map_208, __xlx_offset_byte_param_output_feature_map_209, __xlx_offset_byte_param_output_feature_map_210, __xlx_offset_byte_param_output_feature_map_211, __xlx_offset_byte_param_output_feature_map_212, __xlx_offset_byte_param_output_feature_map_213, __xlx_offset_byte_param_output_feature_map_214, __xlx_offset_byte_param_output_feature_map_215, __xlx_offset_byte_param_output_feature_map_216, __xlx_offset_byte_param_output_feature_map_217, __xlx_offset_byte_param_output_feature_map_218, __xlx_offset_byte_param_output_feature_map_219, __xlx_offset_byte_param_output_feature_map_220, __xlx_offset_byte_param_output_feature_map_221, __xlx_offset_byte_param_output_feature_map_222, __xlx_offset_byte_param_output_feature_map_223, __xlx_offset_byte_param_output_feature_map_224, __xlx_offset_byte_param_output_feature_map_225, __xlx_offset_byte_param_output_feature_map_226, __xlx_offset_byte_param_output_feature_map_227, __xlx_offset_byte_param_output_feature_map_228, __xlx_offset_byte_param_output_feature_map_229, __xlx_offset_byte_param_output_feature_map_230, __xlx_offset_byte_param_output_feature_map_231, __xlx_offset_byte_param_output_feature_map_232, __xlx_offset_byte_param_output_feature_map_233, __xlx_offset_byte_param_output_feature_map_234, __xlx_offset_byte_param_output_feature_map_235, __xlx_offset_byte_param_output_feature_map_236, __xlx_offset_byte_param_output_feature_map_237, __xlx_offset_byte_param_output_feature_map_238, __xlx_offset_byte_param_output_feature_map_239, __xlx_offset_byte_param_output_feature_map_240, __xlx_offset_byte_param_output_feature_map_241, __xlx_offset_byte_param_output_feature_map_242, __xlx_offset_byte_param_output_feature_map_243, __xlx_offset_byte_param_output_feature_map_244, __xlx_offset_byte_param_output_feature_map_245, __xlx_offset_byte_param_output_feature_map_246, __xlx_offset_byte_param_output_feature_map_247, __xlx_offset_byte_param_output_feature_map_248, __xlx_offset_byte_param_output_feature_map_249, __xlx_offset_byte_param_output_feature_map_250, __xlx_offset_byte_param_output_feature_map_251, __xlx_offset_byte_param_output_feature_map_252, __xlx_offset_byte_param_output_feature_map_253, __xlx_offset_byte_param_output_feature_map_254, __xlx_offset_byte_param_output_feature_map_255, __xlx_offset_byte_param_output_feature_map_256, __xlx_offset_byte_param_output_feature_map_257, __xlx_offset_byte_param_output_feature_map_258, __xlx_offset_byte_param_output_feature_map_259, __xlx_offset_byte_param_output_feature_map_260, __xlx_offset_byte_param_output_feature_map_261, __xlx_offset_byte_param_output_feature_map_262, __xlx_offset_byte_param_output_feature_map_263, __xlx_offset_byte_param_output_feature_map_264, __xlx_offset_byte_param_output_feature_map_265, __xlx_offset_byte_param_output_feature_map_266, __xlx_offset_byte_param_output_feature_map_267, __xlx_offset_byte_param_output_feature_map_268, __xlx_offset_byte_param_output_feature_map_269, __xlx_offset_byte_param_output_feature_map_270, __xlx_offset_byte_param_output_feature_map_271, __xlx_offset_byte_param_output_feature_map_272, __xlx_offset_byte_param_output_feature_map_273, __xlx_offset_byte_param_output_feature_map_274, __xlx_offset_byte_param_output_feature_map_275, __xlx_offset_byte_param_output_feature_map_276, __xlx_offset_byte_param_output_feature_map_277, __xlx_offset_byte_param_output_feature_map_278, __xlx_offset_byte_param_output_feature_map_279, __xlx_offset_byte_param_output_feature_map_280, __xlx_offset_byte_param_output_feature_map_281, __xlx_offset_byte_param_output_feature_map_282, __xlx_offset_byte_param_output_feature_map_283, __xlx_offset_byte_param_output_feature_map_284, __xlx_offset_byte_param_output_feature_map_285, __xlx_offset_byte_param_output_feature_map_286, __xlx_offset_byte_param_output_feature_map_287, __xlx_offset_byte_param_output_feature_map_288, __xlx_offset_byte_param_output_feature_map_289, __xlx_offset_byte_param_output_feature_map_290, __xlx_offset_byte_param_output_feature_map_291, __xlx_offset_byte_param_output_feature_map_292, __xlx_offset_byte_param_output_feature_map_293, __xlx_offset_byte_param_output_feature_map_294, __xlx_offset_byte_param_output_feature_map_295, __xlx_offset_byte_param_output_feature_map_296, __xlx_offset_byte_param_output_feature_map_297, __xlx_offset_byte_param_output_feature_map_298, __xlx_offset_byte_param_output_feature_map_299, __xlx_offset_byte_param_output_feature_map_300, __xlx_offset_byte_param_output_feature_map_301, __xlx_offset_byte_param_output_feature_map_302, __xlx_offset_byte_param_output_feature_map_303, __xlx_offset_byte_param_output_feature_map_304, __xlx_offset_byte_param_output_feature_map_305, __xlx_offset_byte_param_output_feature_map_306, __xlx_offset_byte_param_output_feature_map_307, __xlx_offset_byte_param_output_feature_map_308, __xlx_offset_byte_param_output_feature_map_309, __xlx_offset_byte_param_output_feature_map_310, __xlx_offset_byte_param_output_feature_map_311, __xlx_offset_byte_param_output_feature_map_312, __xlx_offset_byte_param_output_feature_map_313, __xlx_offset_byte_param_output_feature_map_314, __xlx_offset_byte_param_output_feature_map_315, __xlx_offset_byte_param_output_feature_map_316, __xlx_offset_byte_param_output_feature_map_317, __xlx_offset_byte_param_output_feature_map_318, __xlx_offset_byte_param_output_feature_map_319);
// print __xlx_apatb_param_input_feature_map
for (size_t i = 0; i < __xlx_size_param_input_feature_map; ++i) {
((Byte<2>*)__xlx_apatb_param_input_feature_map)[i] = __xlx_input_feature_map__tmp_vec[__xlx_offset_param_input_feature_map+i];
}
// print __xlx_apatb_param_layer_weights
for (size_t i = 0; i < __xlx_size_param_layer_weights; ++i) {
((Byte<2>*)__xlx_apatb_param_layer_weights)[i] = __xlx_layer_weights_layer_bias__tmp_vec[__xlx_offset_param_layer_weights+i];
}
// print __xlx_apatb_param_layer_bias
for (size_t i = 0; i < __xlx_size_param_layer_bias; ++i) {
((Byte<2>*)__xlx_apatb_param_layer_bias)[i] = __xlx_layer_weights_layer_bias__tmp_vec[__xlx_offset_param_layer_bias+i];
}
// print __xlx_apatb_param_output_feature_map_0
for (size_t i = 0; i < __xlx_size_param_output_feature_map_0; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_0)[i] = __xlx_output_feature_map_0__tmp_vec[__xlx_offset_param_output_feature_map_0+i];
}
// print __xlx_apatb_param_output_feature_map_1
for (size_t i = 0; i < __xlx_size_param_output_feature_map_1; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_1)[i] = __xlx_output_feature_map_1__tmp_vec[__xlx_offset_param_output_feature_map_1+i];
}
// print __xlx_apatb_param_output_feature_map_2
for (size_t i = 0; i < __xlx_size_param_output_feature_map_2; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_2)[i] = __xlx_output_feature_map_2__tmp_vec[__xlx_offset_param_output_feature_map_2+i];
}
// print __xlx_apatb_param_output_feature_map_3
for (size_t i = 0; i < __xlx_size_param_output_feature_map_3; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_3)[i] = __xlx_output_feature_map_3__tmp_vec[__xlx_offset_param_output_feature_map_3+i];
}
// print __xlx_apatb_param_output_feature_map_4
for (size_t i = 0; i < __xlx_size_param_output_feature_map_4; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_4)[i] = __xlx_output_feature_map_4__tmp_vec[__xlx_offset_param_output_feature_map_4+i];
}
// print __xlx_apatb_param_output_feature_map_5
for (size_t i = 0; i < __xlx_size_param_output_feature_map_5; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_5)[i] = __xlx_output_feature_map_5__tmp_vec[__xlx_offset_param_output_feature_map_5+i];
}
// print __xlx_apatb_param_output_feature_map_6
for (size_t i = 0; i < __xlx_size_param_output_feature_map_6; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_6)[i] = __xlx_output_feature_map_6__tmp_vec[__xlx_offset_param_output_feature_map_6+i];
}
// print __xlx_apatb_param_output_feature_map_7
for (size_t i = 0; i < __xlx_size_param_output_feature_map_7; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_7)[i] = __xlx_output_feature_map_7__tmp_vec[__xlx_offset_param_output_feature_map_7+i];
}
// print __xlx_apatb_param_output_feature_map_8
for (size_t i = 0; i < __xlx_size_param_output_feature_map_8; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_8)[i] = __xlx_output_feature_map_8__tmp_vec[__xlx_offset_param_output_feature_map_8+i];
}
// print __xlx_apatb_param_output_feature_map_9
for (size_t i = 0; i < __xlx_size_param_output_feature_map_9; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_9)[i] = __xlx_output_feature_map_9__tmp_vec[__xlx_offset_param_output_feature_map_9+i];
}
// print __xlx_apatb_param_output_feature_map_10
for (size_t i = 0; i < __xlx_size_param_output_feature_map_10; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_10)[i] = __xlx_output_feature_map_10__tmp_vec[__xlx_offset_param_output_feature_map_10+i];
}
// print __xlx_apatb_param_output_feature_map_11
for (size_t i = 0; i < __xlx_size_param_output_feature_map_11; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_11)[i] = __xlx_output_feature_map_11__tmp_vec[__xlx_offset_param_output_feature_map_11+i];
}
// print __xlx_apatb_param_output_feature_map_12
for (size_t i = 0; i < __xlx_size_param_output_feature_map_12; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_12)[i] = __xlx_output_feature_map_12__tmp_vec[__xlx_offset_param_output_feature_map_12+i];
}
// print __xlx_apatb_param_output_feature_map_13
for (size_t i = 0; i < __xlx_size_param_output_feature_map_13; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_13)[i] = __xlx_output_feature_map_13__tmp_vec[__xlx_offset_param_output_feature_map_13+i];
}
// print __xlx_apatb_param_output_feature_map_14
for (size_t i = 0; i < __xlx_size_param_output_feature_map_14; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_14)[i] = __xlx_output_feature_map_14__tmp_vec[__xlx_offset_param_output_feature_map_14+i];
}
// print __xlx_apatb_param_output_feature_map_15
for (size_t i = 0; i < __xlx_size_param_output_feature_map_15; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_15)[i] = __xlx_output_feature_map_15__tmp_vec[__xlx_offset_param_output_feature_map_15+i];
}
// print __xlx_apatb_param_output_feature_map_16
for (size_t i = 0; i < __xlx_size_param_output_feature_map_16; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_16)[i] = __xlx_output_feature_map_16__tmp_vec[__xlx_offset_param_output_feature_map_16+i];
}
// print __xlx_apatb_param_output_feature_map_17
for (size_t i = 0; i < __xlx_size_param_output_feature_map_17; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_17)[i] = __xlx_output_feature_map_17__tmp_vec[__xlx_offset_param_output_feature_map_17+i];
}
// print __xlx_apatb_param_output_feature_map_18
for (size_t i = 0; i < __xlx_size_param_output_feature_map_18; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_18)[i] = __xlx_output_feature_map_18__tmp_vec[__xlx_offset_param_output_feature_map_18+i];
}
// print __xlx_apatb_param_output_feature_map_19
for (size_t i = 0; i < __xlx_size_param_output_feature_map_19; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_19)[i] = __xlx_output_feature_map_19__tmp_vec[__xlx_offset_param_output_feature_map_19+i];
}
// print __xlx_apatb_param_output_feature_map_20
for (size_t i = 0; i < __xlx_size_param_output_feature_map_20; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_20)[i] = __xlx_output_feature_map_20__tmp_vec[__xlx_offset_param_output_feature_map_20+i];
}
// print __xlx_apatb_param_output_feature_map_21
for (size_t i = 0; i < __xlx_size_param_output_feature_map_21; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_21)[i] = __xlx_output_feature_map_21__tmp_vec[__xlx_offset_param_output_feature_map_21+i];
}
// print __xlx_apatb_param_output_feature_map_22
for (size_t i = 0; i < __xlx_size_param_output_feature_map_22; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_22)[i] = __xlx_output_feature_map_22__tmp_vec[__xlx_offset_param_output_feature_map_22+i];
}
// print __xlx_apatb_param_output_feature_map_23
for (size_t i = 0; i < __xlx_size_param_output_feature_map_23; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_23)[i] = __xlx_output_feature_map_23__tmp_vec[__xlx_offset_param_output_feature_map_23+i];
}
// print __xlx_apatb_param_output_feature_map_24
for (size_t i = 0; i < __xlx_size_param_output_feature_map_24; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_24)[i] = __xlx_output_feature_map_24__tmp_vec[__xlx_offset_param_output_feature_map_24+i];
}
// print __xlx_apatb_param_output_feature_map_25
for (size_t i = 0; i < __xlx_size_param_output_feature_map_25; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_25)[i] = __xlx_output_feature_map_25__tmp_vec[__xlx_offset_param_output_feature_map_25+i];
}
// print __xlx_apatb_param_output_feature_map_26
for (size_t i = 0; i < __xlx_size_param_output_feature_map_26; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_26)[i] = __xlx_output_feature_map_26__tmp_vec[__xlx_offset_param_output_feature_map_26+i];
}
// print __xlx_apatb_param_output_feature_map_27
for (size_t i = 0; i < __xlx_size_param_output_feature_map_27; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_27)[i] = __xlx_output_feature_map_27__tmp_vec[__xlx_offset_param_output_feature_map_27+i];
}
// print __xlx_apatb_param_output_feature_map_28
for (size_t i = 0; i < __xlx_size_param_output_feature_map_28; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_28)[i] = __xlx_output_feature_map_28__tmp_vec[__xlx_offset_param_output_feature_map_28+i];
}
// print __xlx_apatb_param_output_feature_map_29
for (size_t i = 0; i < __xlx_size_param_output_feature_map_29; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_29)[i] = __xlx_output_feature_map_29__tmp_vec[__xlx_offset_param_output_feature_map_29+i];
}
// print __xlx_apatb_param_output_feature_map_30
for (size_t i = 0; i < __xlx_size_param_output_feature_map_30; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_30)[i] = __xlx_output_feature_map_30__tmp_vec[__xlx_offset_param_output_feature_map_30+i];
}
// print __xlx_apatb_param_output_feature_map_31
for (size_t i = 0; i < __xlx_size_param_output_feature_map_31; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_31)[i] = __xlx_output_feature_map_31__tmp_vec[__xlx_offset_param_output_feature_map_31+i];
}
// print __xlx_apatb_param_output_feature_map_32
for (size_t i = 0; i < __xlx_size_param_output_feature_map_32; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_32)[i] = __xlx_output_feature_map_32__tmp_vec[__xlx_offset_param_output_feature_map_32+i];
}
// print __xlx_apatb_param_output_feature_map_33
for (size_t i = 0; i < __xlx_size_param_output_feature_map_33; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_33)[i] = __xlx_output_feature_map_33__tmp_vec[__xlx_offset_param_output_feature_map_33+i];
}
// print __xlx_apatb_param_output_feature_map_34
for (size_t i = 0; i < __xlx_size_param_output_feature_map_34; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_34)[i] = __xlx_output_feature_map_34__tmp_vec[__xlx_offset_param_output_feature_map_34+i];
}
// print __xlx_apatb_param_output_feature_map_35
for (size_t i = 0; i < __xlx_size_param_output_feature_map_35; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_35)[i] = __xlx_output_feature_map_35__tmp_vec[__xlx_offset_param_output_feature_map_35+i];
}
// print __xlx_apatb_param_output_feature_map_36
for (size_t i = 0; i < __xlx_size_param_output_feature_map_36; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_36)[i] = __xlx_output_feature_map_36__tmp_vec[__xlx_offset_param_output_feature_map_36+i];
}
// print __xlx_apatb_param_output_feature_map_37
for (size_t i = 0; i < __xlx_size_param_output_feature_map_37; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_37)[i] = __xlx_output_feature_map_37__tmp_vec[__xlx_offset_param_output_feature_map_37+i];
}
// print __xlx_apatb_param_output_feature_map_38
for (size_t i = 0; i < __xlx_size_param_output_feature_map_38; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_38)[i] = __xlx_output_feature_map_38__tmp_vec[__xlx_offset_param_output_feature_map_38+i];
}
// print __xlx_apatb_param_output_feature_map_39
for (size_t i = 0; i < __xlx_size_param_output_feature_map_39; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_39)[i] = __xlx_output_feature_map_39__tmp_vec[__xlx_offset_param_output_feature_map_39+i];
}
// print __xlx_apatb_param_output_feature_map_40
for (size_t i = 0; i < __xlx_size_param_output_feature_map_40; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_40)[i] = __xlx_output_feature_map_40__tmp_vec[__xlx_offset_param_output_feature_map_40+i];
}
// print __xlx_apatb_param_output_feature_map_41
for (size_t i = 0; i < __xlx_size_param_output_feature_map_41; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_41)[i] = __xlx_output_feature_map_41__tmp_vec[__xlx_offset_param_output_feature_map_41+i];
}
// print __xlx_apatb_param_output_feature_map_42
for (size_t i = 0; i < __xlx_size_param_output_feature_map_42; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_42)[i] = __xlx_output_feature_map_42__tmp_vec[__xlx_offset_param_output_feature_map_42+i];
}
// print __xlx_apatb_param_output_feature_map_43
for (size_t i = 0; i < __xlx_size_param_output_feature_map_43; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_43)[i] = __xlx_output_feature_map_43__tmp_vec[__xlx_offset_param_output_feature_map_43+i];
}
// print __xlx_apatb_param_output_feature_map_44
for (size_t i = 0; i < __xlx_size_param_output_feature_map_44; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_44)[i] = __xlx_output_feature_map_44__tmp_vec[__xlx_offset_param_output_feature_map_44+i];
}
// print __xlx_apatb_param_output_feature_map_45
for (size_t i = 0; i < __xlx_size_param_output_feature_map_45; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_45)[i] = __xlx_output_feature_map_45__tmp_vec[__xlx_offset_param_output_feature_map_45+i];
}
// print __xlx_apatb_param_output_feature_map_46
for (size_t i = 0; i < __xlx_size_param_output_feature_map_46; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_46)[i] = __xlx_output_feature_map_46__tmp_vec[__xlx_offset_param_output_feature_map_46+i];
}
// print __xlx_apatb_param_output_feature_map_47
for (size_t i = 0; i < __xlx_size_param_output_feature_map_47; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_47)[i] = __xlx_output_feature_map_47__tmp_vec[__xlx_offset_param_output_feature_map_47+i];
}
// print __xlx_apatb_param_output_feature_map_48
for (size_t i = 0; i < __xlx_size_param_output_feature_map_48; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_48)[i] = __xlx_output_feature_map_48__tmp_vec[__xlx_offset_param_output_feature_map_48+i];
}
// print __xlx_apatb_param_output_feature_map_49
for (size_t i = 0; i < __xlx_size_param_output_feature_map_49; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_49)[i] = __xlx_output_feature_map_49__tmp_vec[__xlx_offset_param_output_feature_map_49+i];
}
// print __xlx_apatb_param_output_feature_map_50
for (size_t i = 0; i < __xlx_size_param_output_feature_map_50; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_50)[i] = __xlx_output_feature_map_50__tmp_vec[__xlx_offset_param_output_feature_map_50+i];
}
// print __xlx_apatb_param_output_feature_map_51
for (size_t i = 0; i < __xlx_size_param_output_feature_map_51; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_51)[i] = __xlx_output_feature_map_51__tmp_vec[__xlx_offset_param_output_feature_map_51+i];
}
// print __xlx_apatb_param_output_feature_map_52
for (size_t i = 0; i < __xlx_size_param_output_feature_map_52; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_52)[i] = __xlx_output_feature_map_52__tmp_vec[__xlx_offset_param_output_feature_map_52+i];
}
// print __xlx_apatb_param_output_feature_map_53
for (size_t i = 0; i < __xlx_size_param_output_feature_map_53; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_53)[i] = __xlx_output_feature_map_53__tmp_vec[__xlx_offset_param_output_feature_map_53+i];
}
// print __xlx_apatb_param_output_feature_map_54
for (size_t i = 0; i < __xlx_size_param_output_feature_map_54; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_54)[i] = __xlx_output_feature_map_54__tmp_vec[__xlx_offset_param_output_feature_map_54+i];
}
// print __xlx_apatb_param_output_feature_map_55
for (size_t i = 0; i < __xlx_size_param_output_feature_map_55; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_55)[i] = __xlx_output_feature_map_55__tmp_vec[__xlx_offset_param_output_feature_map_55+i];
}
// print __xlx_apatb_param_output_feature_map_56
for (size_t i = 0; i < __xlx_size_param_output_feature_map_56; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_56)[i] = __xlx_output_feature_map_56__tmp_vec[__xlx_offset_param_output_feature_map_56+i];
}
// print __xlx_apatb_param_output_feature_map_57
for (size_t i = 0; i < __xlx_size_param_output_feature_map_57; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_57)[i] = __xlx_output_feature_map_57__tmp_vec[__xlx_offset_param_output_feature_map_57+i];
}
// print __xlx_apatb_param_output_feature_map_58
for (size_t i = 0; i < __xlx_size_param_output_feature_map_58; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_58)[i] = __xlx_output_feature_map_58__tmp_vec[__xlx_offset_param_output_feature_map_58+i];
}
// print __xlx_apatb_param_output_feature_map_59
for (size_t i = 0; i < __xlx_size_param_output_feature_map_59; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_59)[i] = __xlx_output_feature_map_59__tmp_vec[__xlx_offset_param_output_feature_map_59+i];
}
// print __xlx_apatb_param_output_feature_map_60
for (size_t i = 0; i < __xlx_size_param_output_feature_map_60; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_60)[i] = __xlx_output_feature_map_60__tmp_vec[__xlx_offset_param_output_feature_map_60+i];
}
// print __xlx_apatb_param_output_feature_map_61
for (size_t i = 0; i < __xlx_size_param_output_feature_map_61; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_61)[i] = __xlx_output_feature_map_61__tmp_vec[__xlx_offset_param_output_feature_map_61+i];
}
// print __xlx_apatb_param_output_feature_map_62
for (size_t i = 0; i < __xlx_size_param_output_feature_map_62; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_62)[i] = __xlx_output_feature_map_62__tmp_vec[__xlx_offset_param_output_feature_map_62+i];
}
// print __xlx_apatb_param_output_feature_map_63
for (size_t i = 0; i < __xlx_size_param_output_feature_map_63; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_63)[i] = __xlx_output_feature_map_63__tmp_vec[__xlx_offset_param_output_feature_map_63+i];
}
// print __xlx_apatb_param_output_feature_map_64
for (size_t i = 0; i < __xlx_size_param_output_feature_map_64; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_64)[i] = __xlx_output_feature_map_64__tmp_vec[__xlx_offset_param_output_feature_map_64+i];
}
// print __xlx_apatb_param_output_feature_map_65
for (size_t i = 0; i < __xlx_size_param_output_feature_map_65; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_65)[i] = __xlx_output_feature_map_65__tmp_vec[__xlx_offset_param_output_feature_map_65+i];
}
// print __xlx_apatb_param_output_feature_map_66
for (size_t i = 0; i < __xlx_size_param_output_feature_map_66; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_66)[i] = __xlx_output_feature_map_66__tmp_vec[__xlx_offset_param_output_feature_map_66+i];
}
// print __xlx_apatb_param_output_feature_map_67
for (size_t i = 0; i < __xlx_size_param_output_feature_map_67; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_67)[i] = __xlx_output_feature_map_67__tmp_vec[__xlx_offset_param_output_feature_map_67+i];
}
// print __xlx_apatb_param_output_feature_map_68
for (size_t i = 0; i < __xlx_size_param_output_feature_map_68; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_68)[i] = __xlx_output_feature_map_68__tmp_vec[__xlx_offset_param_output_feature_map_68+i];
}
// print __xlx_apatb_param_output_feature_map_69
for (size_t i = 0; i < __xlx_size_param_output_feature_map_69; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_69)[i] = __xlx_output_feature_map_69__tmp_vec[__xlx_offset_param_output_feature_map_69+i];
}
// print __xlx_apatb_param_output_feature_map_70
for (size_t i = 0; i < __xlx_size_param_output_feature_map_70; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_70)[i] = __xlx_output_feature_map_70__tmp_vec[__xlx_offset_param_output_feature_map_70+i];
}
// print __xlx_apatb_param_output_feature_map_71
for (size_t i = 0; i < __xlx_size_param_output_feature_map_71; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_71)[i] = __xlx_output_feature_map_71__tmp_vec[__xlx_offset_param_output_feature_map_71+i];
}
// print __xlx_apatb_param_output_feature_map_72
for (size_t i = 0; i < __xlx_size_param_output_feature_map_72; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_72)[i] = __xlx_output_feature_map_72__tmp_vec[__xlx_offset_param_output_feature_map_72+i];
}
// print __xlx_apatb_param_output_feature_map_73
for (size_t i = 0; i < __xlx_size_param_output_feature_map_73; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_73)[i] = __xlx_output_feature_map_73__tmp_vec[__xlx_offset_param_output_feature_map_73+i];
}
// print __xlx_apatb_param_output_feature_map_74
for (size_t i = 0; i < __xlx_size_param_output_feature_map_74; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_74)[i] = __xlx_output_feature_map_74__tmp_vec[__xlx_offset_param_output_feature_map_74+i];
}
// print __xlx_apatb_param_output_feature_map_75
for (size_t i = 0; i < __xlx_size_param_output_feature_map_75; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_75)[i] = __xlx_output_feature_map_75__tmp_vec[__xlx_offset_param_output_feature_map_75+i];
}
// print __xlx_apatb_param_output_feature_map_76
for (size_t i = 0; i < __xlx_size_param_output_feature_map_76; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_76)[i] = __xlx_output_feature_map_76__tmp_vec[__xlx_offset_param_output_feature_map_76+i];
}
// print __xlx_apatb_param_output_feature_map_77
for (size_t i = 0; i < __xlx_size_param_output_feature_map_77; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_77)[i] = __xlx_output_feature_map_77__tmp_vec[__xlx_offset_param_output_feature_map_77+i];
}
// print __xlx_apatb_param_output_feature_map_78
for (size_t i = 0; i < __xlx_size_param_output_feature_map_78; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_78)[i] = __xlx_output_feature_map_78__tmp_vec[__xlx_offset_param_output_feature_map_78+i];
}
// print __xlx_apatb_param_output_feature_map_79
for (size_t i = 0; i < __xlx_size_param_output_feature_map_79; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_79)[i] = __xlx_output_feature_map_79__tmp_vec[__xlx_offset_param_output_feature_map_79+i];
}
// print __xlx_apatb_param_output_feature_map_80
for (size_t i = 0; i < __xlx_size_param_output_feature_map_80; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_80)[i] = __xlx_output_feature_map_80__tmp_vec[__xlx_offset_param_output_feature_map_80+i];
}
// print __xlx_apatb_param_output_feature_map_81
for (size_t i = 0; i < __xlx_size_param_output_feature_map_81; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_81)[i] = __xlx_output_feature_map_81__tmp_vec[__xlx_offset_param_output_feature_map_81+i];
}
// print __xlx_apatb_param_output_feature_map_82
for (size_t i = 0; i < __xlx_size_param_output_feature_map_82; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_82)[i] = __xlx_output_feature_map_82__tmp_vec[__xlx_offset_param_output_feature_map_82+i];
}
// print __xlx_apatb_param_output_feature_map_83
for (size_t i = 0; i < __xlx_size_param_output_feature_map_83; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_83)[i] = __xlx_output_feature_map_83__tmp_vec[__xlx_offset_param_output_feature_map_83+i];
}
// print __xlx_apatb_param_output_feature_map_84
for (size_t i = 0; i < __xlx_size_param_output_feature_map_84; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_84)[i] = __xlx_output_feature_map_84__tmp_vec[__xlx_offset_param_output_feature_map_84+i];
}
// print __xlx_apatb_param_output_feature_map_85
for (size_t i = 0; i < __xlx_size_param_output_feature_map_85; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_85)[i] = __xlx_output_feature_map_85__tmp_vec[__xlx_offset_param_output_feature_map_85+i];
}
// print __xlx_apatb_param_output_feature_map_86
for (size_t i = 0; i < __xlx_size_param_output_feature_map_86; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_86)[i] = __xlx_output_feature_map_86__tmp_vec[__xlx_offset_param_output_feature_map_86+i];
}
// print __xlx_apatb_param_output_feature_map_87
for (size_t i = 0; i < __xlx_size_param_output_feature_map_87; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_87)[i] = __xlx_output_feature_map_87__tmp_vec[__xlx_offset_param_output_feature_map_87+i];
}
// print __xlx_apatb_param_output_feature_map_88
for (size_t i = 0; i < __xlx_size_param_output_feature_map_88; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_88)[i] = __xlx_output_feature_map_88__tmp_vec[__xlx_offset_param_output_feature_map_88+i];
}
// print __xlx_apatb_param_output_feature_map_89
for (size_t i = 0; i < __xlx_size_param_output_feature_map_89; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_89)[i] = __xlx_output_feature_map_89__tmp_vec[__xlx_offset_param_output_feature_map_89+i];
}
// print __xlx_apatb_param_output_feature_map_90
for (size_t i = 0; i < __xlx_size_param_output_feature_map_90; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_90)[i] = __xlx_output_feature_map_90__tmp_vec[__xlx_offset_param_output_feature_map_90+i];
}
// print __xlx_apatb_param_output_feature_map_91
for (size_t i = 0; i < __xlx_size_param_output_feature_map_91; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_91)[i] = __xlx_output_feature_map_91__tmp_vec[__xlx_offset_param_output_feature_map_91+i];
}
// print __xlx_apatb_param_output_feature_map_92
for (size_t i = 0; i < __xlx_size_param_output_feature_map_92; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_92)[i] = __xlx_output_feature_map_92__tmp_vec[__xlx_offset_param_output_feature_map_92+i];
}
// print __xlx_apatb_param_output_feature_map_93
for (size_t i = 0; i < __xlx_size_param_output_feature_map_93; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_93)[i] = __xlx_output_feature_map_93__tmp_vec[__xlx_offset_param_output_feature_map_93+i];
}
// print __xlx_apatb_param_output_feature_map_94
for (size_t i = 0; i < __xlx_size_param_output_feature_map_94; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_94)[i] = __xlx_output_feature_map_94__tmp_vec[__xlx_offset_param_output_feature_map_94+i];
}
// print __xlx_apatb_param_output_feature_map_95
for (size_t i = 0; i < __xlx_size_param_output_feature_map_95; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_95)[i] = __xlx_output_feature_map_95__tmp_vec[__xlx_offset_param_output_feature_map_95+i];
}
// print __xlx_apatb_param_output_feature_map_96
for (size_t i = 0; i < __xlx_size_param_output_feature_map_96; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_96)[i] = __xlx_output_feature_map_96__tmp_vec[__xlx_offset_param_output_feature_map_96+i];
}
// print __xlx_apatb_param_output_feature_map_97
for (size_t i = 0; i < __xlx_size_param_output_feature_map_97; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_97)[i] = __xlx_output_feature_map_97__tmp_vec[__xlx_offset_param_output_feature_map_97+i];
}
// print __xlx_apatb_param_output_feature_map_98
for (size_t i = 0; i < __xlx_size_param_output_feature_map_98; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_98)[i] = __xlx_output_feature_map_98__tmp_vec[__xlx_offset_param_output_feature_map_98+i];
}
// print __xlx_apatb_param_output_feature_map_99
for (size_t i = 0; i < __xlx_size_param_output_feature_map_99; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_99)[i] = __xlx_output_feature_map_99__tmp_vec[__xlx_offset_param_output_feature_map_99+i];
}
// print __xlx_apatb_param_output_feature_map_100
for (size_t i = 0; i < __xlx_size_param_output_feature_map_100; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_100)[i] = __xlx_output_feature_map_100__tmp_vec[__xlx_offset_param_output_feature_map_100+i];
}
// print __xlx_apatb_param_output_feature_map_101
for (size_t i = 0; i < __xlx_size_param_output_feature_map_101; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_101)[i] = __xlx_output_feature_map_101__tmp_vec[__xlx_offset_param_output_feature_map_101+i];
}
// print __xlx_apatb_param_output_feature_map_102
for (size_t i = 0; i < __xlx_size_param_output_feature_map_102; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_102)[i] = __xlx_output_feature_map_102__tmp_vec[__xlx_offset_param_output_feature_map_102+i];
}
// print __xlx_apatb_param_output_feature_map_103
for (size_t i = 0; i < __xlx_size_param_output_feature_map_103; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_103)[i] = __xlx_output_feature_map_103__tmp_vec[__xlx_offset_param_output_feature_map_103+i];
}
// print __xlx_apatb_param_output_feature_map_104
for (size_t i = 0; i < __xlx_size_param_output_feature_map_104; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_104)[i] = __xlx_output_feature_map_104__tmp_vec[__xlx_offset_param_output_feature_map_104+i];
}
// print __xlx_apatb_param_output_feature_map_105
for (size_t i = 0; i < __xlx_size_param_output_feature_map_105; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_105)[i] = __xlx_output_feature_map_105__tmp_vec[__xlx_offset_param_output_feature_map_105+i];
}
// print __xlx_apatb_param_output_feature_map_106
for (size_t i = 0; i < __xlx_size_param_output_feature_map_106; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_106)[i] = __xlx_output_feature_map_106__tmp_vec[__xlx_offset_param_output_feature_map_106+i];
}
// print __xlx_apatb_param_output_feature_map_107
for (size_t i = 0; i < __xlx_size_param_output_feature_map_107; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_107)[i] = __xlx_output_feature_map_107__tmp_vec[__xlx_offset_param_output_feature_map_107+i];
}
// print __xlx_apatb_param_output_feature_map_108
for (size_t i = 0; i < __xlx_size_param_output_feature_map_108; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_108)[i] = __xlx_output_feature_map_108__tmp_vec[__xlx_offset_param_output_feature_map_108+i];
}
// print __xlx_apatb_param_output_feature_map_109
for (size_t i = 0; i < __xlx_size_param_output_feature_map_109; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_109)[i] = __xlx_output_feature_map_109__tmp_vec[__xlx_offset_param_output_feature_map_109+i];
}
// print __xlx_apatb_param_output_feature_map_110
for (size_t i = 0; i < __xlx_size_param_output_feature_map_110; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_110)[i] = __xlx_output_feature_map_110__tmp_vec[__xlx_offset_param_output_feature_map_110+i];
}
// print __xlx_apatb_param_output_feature_map_111
for (size_t i = 0; i < __xlx_size_param_output_feature_map_111; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_111)[i] = __xlx_output_feature_map_111__tmp_vec[__xlx_offset_param_output_feature_map_111+i];
}
// print __xlx_apatb_param_output_feature_map_112
for (size_t i = 0; i < __xlx_size_param_output_feature_map_112; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_112)[i] = __xlx_output_feature_map_112__tmp_vec[__xlx_offset_param_output_feature_map_112+i];
}
// print __xlx_apatb_param_output_feature_map_113
for (size_t i = 0; i < __xlx_size_param_output_feature_map_113; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_113)[i] = __xlx_output_feature_map_113__tmp_vec[__xlx_offset_param_output_feature_map_113+i];
}
// print __xlx_apatb_param_output_feature_map_114
for (size_t i = 0; i < __xlx_size_param_output_feature_map_114; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_114)[i] = __xlx_output_feature_map_114__tmp_vec[__xlx_offset_param_output_feature_map_114+i];
}
// print __xlx_apatb_param_output_feature_map_115
for (size_t i = 0; i < __xlx_size_param_output_feature_map_115; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_115)[i] = __xlx_output_feature_map_115__tmp_vec[__xlx_offset_param_output_feature_map_115+i];
}
// print __xlx_apatb_param_output_feature_map_116
for (size_t i = 0; i < __xlx_size_param_output_feature_map_116; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_116)[i] = __xlx_output_feature_map_116__tmp_vec[__xlx_offset_param_output_feature_map_116+i];
}
// print __xlx_apatb_param_output_feature_map_117
for (size_t i = 0; i < __xlx_size_param_output_feature_map_117; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_117)[i] = __xlx_output_feature_map_117__tmp_vec[__xlx_offset_param_output_feature_map_117+i];
}
// print __xlx_apatb_param_output_feature_map_118
for (size_t i = 0; i < __xlx_size_param_output_feature_map_118; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_118)[i] = __xlx_output_feature_map_118__tmp_vec[__xlx_offset_param_output_feature_map_118+i];
}
// print __xlx_apatb_param_output_feature_map_119
for (size_t i = 0; i < __xlx_size_param_output_feature_map_119; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_119)[i] = __xlx_output_feature_map_119__tmp_vec[__xlx_offset_param_output_feature_map_119+i];
}
// print __xlx_apatb_param_output_feature_map_120
for (size_t i = 0; i < __xlx_size_param_output_feature_map_120; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_120)[i] = __xlx_output_feature_map_120__tmp_vec[__xlx_offset_param_output_feature_map_120+i];
}
// print __xlx_apatb_param_output_feature_map_121
for (size_t i = 0; i < __xlx_size_param_output_feature_map_121; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_121)[i] = __xlx_output_feature_map_121__tmp_vec[__xlx_offset_param_output_feature_map_121+i];
}
// print __xlx_apatb_param_output_feature_map_122
for (size_t i = 0; i < __xlx_size_param_output_feature_map_122; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_122)[i] = __xlx_output_feature_map_122__tmp_vec[__xlx_offset_param_output_feature_map_122+i];
}
// print __xlx_apatb_param_output_feature_map_123
for (size_t i = 0; i < __xlx_size_param_output_feature_map_123; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_123)[i] = __xlx_output_feature_map_123__tmp_vec[__xlx_offset_param_output_feature_map_123+i];
}
// print __xlx_apatb_param_output_feature_map_124
for (size_t i = 0; i < __xlx_size_param_output_feature_map_124; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_124)[i] = __xlx_output_feature_map_124__tmp_vec[__xlx_offset_param_output_feature_map_124+i];
}
// print __xlx_apatb_param_output_feature_map_125
for (size_t i = 0; i < __xlx_size_param_output_feature_map_125; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_125)[i] = __xlx_output_feature_map_125__tmp_vec[__xlx_offset_param_output_feature_map_125+i];
}
// print __xlx_apatb_param_output_feature_map_126
for (size_t i = 0; i < __xlx_size_param_output_feature_map_126; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_126)[i] = __xlx_output_feature_map_126__tmp_vec[__xlx_offset_param_output_feature_map_126+i];
}
// print __xlx_apatb_param_output_feature_map_127
for (size_t i = 0; i < __xlx_size_param_output_feature_map_127; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_127)[i] = __xlx_output_feature_map_127__tmp_vec[__xlx_offset_param_output_feature_map_127+i];
}
// print __xlx_apatb_param_output_feature_map_128
for (size_t i = 0; i < __xlx_size_param_output_feature_map_128; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_128)[i] = __xlx_output_feature_map_128__tmp_vec[__xlx_offset_param_output_feature_map_128+i];
}
// print __xlx_apatb_param_output_feature_map_129
for (size_t i = 0; i < __xlx_size_param_output_feature_map_129; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_129)[i] = __xlx_output_feature_map_129__tmp_vec[__xlx_offset_param_output_feature_map_129+i];
}
// print __xlx_apatb_param_output_feature_map_130
for (size_t i = 0; i < __xlx_size_param_output_feature_map_130; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_130)[i] = __xlx_output_feature_map_130__tmp_vec[__xlx_offset_param_output_feature_map_130+i];
}
// print __xlx_apatb_param_output_feature_map_131
for (size_t i = 0; i < __xlx_size_param_output_feature_map_131; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_131)[i] = __xlx_output_feature_map_131__tmp_vec[__xlx_offset_param_output_feature_map_131+i];
}
// print __xlx_apatb_param_output_feature_map_132
for (size_t i = 0; i < __xlx_size_param_output_feature_map_132; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_132)[i] = __xlx_output_feature_map_132__tmp_vec[__xlx_offset_param_output_feature_map_132+i];
}
// print __xlx_apatb_param_output_feature_map_133
for (size_t i = 0; i < __xlx_size_param_output_feature_map_133; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_133)[i] = __xlx_output_feature_map_133__tmp_vec[__xlx_offset_param_output_feature_map_133+i];
}
// print __xlx_apatb_param_output_feature_map_134
for (size_t i = 0; i < __xlx_size_param_output_feature_map_134; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_134)[i] = __xlx_output_feature_map_134__tmp_vec[__xlx_offset_param_output_feature_map_134+i];
}
// print __xlx_apatb_param_output_feature_map_135
for (size_t i = 0; i < __xlx_size_param_output_feature_map_135; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_135)[i] = __xlx_output_feature_map_135__tmp_vec[__xlx_offset_param_output_feature_map_135+i];
}
// print __xlx_apatb_param_output_feature_map_136
for (size_t i = 0; i < __xlx_size_param_output_feature_map_136; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_136)[i] = __xlx_output_feature_map_136__tmp_vec[__xlx_offset_param_output_feature_map_136+i];
}
// print __xlx_apatb_param_output_feature_map_137
for (size_t i = 0; i < __xlx_size_param_output_feature_map_137; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_137)[i] = __xlx_output_feature_map_137__tmp_vec[__xlx_offset_param_output_feature_map_137+i];
}
// print __xlx_apatb_param_output_feature_map_138
for (size_t i = 0; i < __xlx_size_param_output_feature_map_138; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_138)[i] = __xlx_output_feature_map_138__tmp_vec[__xlx_offset_param_output_feature_map_138+i];
}
// print __xlx_apatb_param_output_feature_map_139
for (size_t i = 0; i < __xlx_size_param_output_feature_map_139; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_139)[i] = __xlx_output_feature_map_139__tmp_vec[__xlx_offset_param_output_feature_map_139+i];
}
// print __xlx_apatb_param_output_feature_map_140
for (size_t i = 0; i < __xlx_size_param_output_feature_map_140; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_140)[i] = __xlx_output_feature_map_140__tmp_vec[__xlx_offset_param_output_feature_map_140+i];
}
// print __xlx_apatb_param_output_feature_map_141
for (size_t i = 0; i < __xlx_size_param_output_feature_map_141; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_141)[i] = __xlx_output_feature_map_141__tmp_vec[__xlx_offset_param_output_feature_map_141+i];
}
// print __xlx_apatb_param_output_feature_map_142
for (size_t i = 0; i < __xlx_size_param_output_feature_map_142; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_142)[i] = __xlx_output_feature_map_142__tmp_vec[__xlx_offset_param_output_feature_map_142+i];
}
// print __xlx_apatb_param_output_feature_map_143
for (size_t i = 0; i < __xlx_size_param_output_feature_map_143; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_143)[i] = __xlx_output_feature_map_143__tmp_vec[__xlx_offset_param_output_feature_map_143+i];
}
// print __xlx_apatb_param_output_feature_map_144
for (size_t i = 0; i < __xlx_size_param_output_feature_map_144; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_144)[i] = __xlx_output_feature_map_144__tmp_vec[__xlx_offset_param_output_feature_map_144+i];
}
// print __xlx_apatb_param_output_feature_map_145
for (size_t i = 0; i < __xlx_size_param_output_feature_map_145; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_145)[i] = __xlx_output_feature_map_145__tmp_vec[__xlx_offset_param_output_feature_map_145+i];
}
// print __xlx_apatb_param_output_feature_map_146
for (size_t i = 0; i < __xlx_size_param_output_feature_map_146; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_146)[i] = __xlx_output_feature_map_146__tmp_vec[__xlx_offset_param_output_feature_map_146+i];
}
// print __xlx_apatb_param_output_feature_map_147
for (size_t i = 0; i < __xlx_size_param_output_feature_map_147; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_147)[i] = __xlx_output_feature_map_147__tmp_vec[__xlx_offset_param_output_feature_map_147+i];
}
// print __xlx_apatb_param_output_feature_map_148
for (size_t i = 0; i < __xlx_size_param_output_feature_map_148; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_148)[i] = __xlx_output_feature_map_148__tmp_vec[__xlx_offset_param_output_feature_map_148+i];
}
// print __xlx_apatb_param_output_feature_map_149
for (size_t i = 0; i < __xlx_size_param_output_feature_map_149; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_149)[i] = __xlx_output_feature_map_149__tmp_vec[__xlx_offset_param_output_feature_map_149+i];
}
// print __xlx_apatb_param_output_feature_map_150
for (size_t i = 0; i < __xlx_size_param_output_feature_map_150; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_150)[i] = __xlx_output_feature_map_150__tmp_vec[__xlx_offset_param_output_feature_map_150+i];
}
// print __xlx_apatb_param_output_feature_map_151
for (size_t i = 0; i < __xlx_size_param_output_feature_map_151; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_151)[i] = __xlx_output_feature_map_151__tmp_vec[__xlx_offset_param_output_feature_map_151+i];
}
// print __xlx_apatb_param_output_feature_map_152
for (size_t i = 0; i < __xlx_size_param_output_feature_map_152; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_152)[i] = __xlx_output_feature_map_152__tmp_vec[__xlx_offset_param_output_feature_map_152+i];
}
// print __xlx_apatb_param_output_feature_map_153
for (size_t i = 0; i < __xlx_size_param_output_feature_map_153; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_153)[i] = __xlx_output_feature_map_153__tmp_vec[__xlx_offset_param_output_feature_map_153+i];
}
// print __xlx_apatb_param_output_feature_map_154
for (size_t i = 0; i < __xlx_size_param_output_feature_map_154; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_154)[i] = __xlx_output_feature_map_154__tmp_vec[__xlx_offset_param_output_feature_map_154+i];
}
// print __xlx_apatb_param_output_feature_map_155
for (size_t i = 0; i < __xlx_size_param_output_feature_map_155; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_155)[i] = __xlx_output_feature_map_155__tmp_vec[__xlx_offset_param_output_feature_map_155+i];
}
// print __xlx_apatb_param_output_feature_map_156
for (size_t i = 0; i < __xlx_size_param_output_feature_map_156; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_156)[i] = __xlx_output_feature_map_156__tmp_vec[__xlx_offset_param_output_feature_map_156+i];
}
// print __xlx_apatb_param_output_feature_map_157
for (size_t i = 0; i < __xlx_size_param_output_feature_map_157; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_157)[i] = __xlx_output_feature_map_157__tmp_vec[__xlx_offset_param_output_feature_map_157+i];
}
// print __xlx_apatb_param_output_feature_map_158
for (size_t i = 0; i < __xlx_size_param_output_feature_map_158; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_158)[i] = __xlx_output_feature_map_158__tmp_vec[__xlx_offset_param_output_feature_map_158+i];
}
// print __xlx_apatb_param_output_feature_map_159
for (size_t i = 0; i < __xlx_size_param_output_feature_map_159; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_159)[i] = __xlx_output_feature_map_159__tmp_vec[__xlx_offset_param_output_feature_map_159+i];
}
// print __xlx_apatb_param_output_feature_map_160
for (size_t i = 0; i < __xlx_size_param_output_feature_map_160; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_160)[i] = __xlx_output_feature_map_160__tmp_vec[__xlx_offset_param_output_feature_map_160+i];
}
// print __xlx_apatb_param_output_feature_map_161
for (size_t i = 0; i < __xlx_size_param_output_feature_map_161; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_161)[i] = __xlx_output_feature_map_161__tmp_vec[__xlx_offset_param_output_feature_map_161+i];
}
// print __xlx_apatb_param_output_feature_map_162
for (size_t i = 0; i < __xlx_size_param_output_feature_map_162; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_162)[i] = __xlx_output_feature_map_162__tmp_vec[__xlx_offset_param_output_feature_map_162+i];
}
// print __xlx_apatb_param_output_feature_map_163
for (size_t i = 0; i < __xlx_size_param_output_feature_map_163; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_163)[i] = __xlx_output_feature_map_163__tmp_vec[__xlx_offset_param_output_feature_map_163+i];
}
// print __xlx_apatb_param_output_feature_map_164
for (size_t i = 0; i < __xlx_size_param_output_feature_map_164; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_164)[i] = __xlx_output_feature_map_164__tmp_vec[__xlx_offset_param_output_feature_map_164+i];
}
// print __xlx_apatb_param_output_feature_map_165
for (size_t i = 0; i < __xlx_size_param_output_feature_map_165; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_165)[i] = __xlx_output_feature_map_165__tmp_vec[__xlx_offset_param_output_feature_map_165+i];
}
// print __xlx_apatb_param_output_feature_map_166
for (size_t i = 0; i < __xlx_size_param_output_feature_map_166; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_166)[i] = __xlx_output_feature_map_166__tmp_vec[__xlx_offset_param_output_feature_map_166+i];
}
// print __xlx_apatb_param_output_feature_map_167
for (size_t i = 0; i < __xlx_size_param_output_feature_map_167; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_167)[i] = __xlx_output_feature_map_167__tmp_vec[__xlx_offset_param_output_feature_map_167+i];
}
// print __xlx_apatb_param_output_feature_map_168
for (size_t i = 0; i < __xlx_size_param_output_feature_map_168; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_168)[i] = __xlx_output_feature_map_168__tmp_vec[__xlx_offset_param_output_feature_map_168+i];
}
// print __xlx_apatb_param_output_feature_map_169
for (size_t i = 0; i < __xlx_size_param_output_feature_map_169; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_169)[i] = __xlx_output_feature_map_169__tmp_vec[__xlx_offset_param_output_feature_map_169+i];
}
// print __xlx_apatb_param_output_feature_map_170
for (size_t i = 0; i < __xlx_size_param_output_feature_map_170; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_170)[i] = __xlx_output_feature_map_170__tmp_vec[__xlx_offset_param_output_feature_map_170+i];
}
// print __xlx_apatb_param_output_feature_map_171
for (size_t i = 0; i < __xlx_size_param_output_feature_map_171; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_171)[i] = __xlx_output_feature_map_171__tmp_vec[__xlx_offset_param_output_feature_map_171+i];
}
// print __xlx_apatb_param_output_feature_map_172
for (size_t i = 0; i < __xlx_size_param_output_feature_map_172; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_172)[i] = __xlx_output_feature_map_172__tmp_vec[__xlx_offset_param_output_feature_map_172+i];
}
// print __xlx_apatb_param_output_feature_map_173
for (size_t i = 0; i < __xlx_size_param_output_feature_map_173; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_173)[i] = __xlx_output_feature_map_173__tmp_vec[__xlx_offset_param_output_feature_map_173+i];
}
// print __xlx_apatb_param_output_feature_map_174
for (size_t i = 0; i < __xlx_size_param_output_feature_map_174; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_174)[i] = __xlx_output_feature_map_174__tmp_vec[__xlx_offset_param_output_feature_map_174+i];
}
// print __xlx_apatb_param_output_feature_map_175
for (size_t i = 0; i < __xlx_size_param_output_feature_map_175; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_175)[i] = __xlx_output_feature_map_175__tmp_vec[__xlx_offset_param_output_feature_map_175+i];
}
// print __xlx_apatb_param_output_feature_map_176
for (size_t i = 0; i < __xlx_size_param_output_feature_map_176; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_176)[i] = __xlx_output_feature_map_176__tmp_vec[__xlx_offset_param_output_feature_map_176+i];
}
// print __xlx_apatb_param_output_feature_map_177
for (size_t i = 0; i < __xlx_size_param_output_feature_map_177; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_177)[i] = __xlx_output_feature_map_177__tmp_vec[__xlx_offset_param_output_feature_map_177+i];
}
// print __xlx_apatb_param_output_feature_map_178
for (size_t i = 0; i < __xlx_size_param_output_feature_map_178; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_178)[i] = __xlx_output_feature_map_178__tmp_vec[__xlx_offset_param_output_feature_map_178+i];
}
// print __xlx_apatb_param_output_feature_map_179
for (size_t i = 0; i < __xlx_size_param_output_feature_map_179; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_179)[i] = __xlx_output_feature_map_179__tmp_vec[__xlx_offset_param_output_feature_map_179+i];
}
// print __xlx_apatb_param_output_feature_map_180
for (size_t i = 0; i < __xlx_size_param_output_feature_map_180; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_180)[i] = __xlx_output_feature_map_180__tmp_vec[__xlx_offset_param_output_feature_map_180+i];
}
// print __xlx_apatb_param_output_feature_map_181
for (size_t i = 0; i < __xlx_size_param_output_feature_map_181; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_181)[i] = __xlx_output_feature_map_181__tmp_vec[__xlx_offset_param_output_feature_map_181+i];
}
// print __xlx_apatb_param_output_feature_map_182
for (size_t i = 0; i < __xlx_size_param_output_feature_map_182; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_182)[i] = __xlx_output_feature_map_182__tmp_vec[__xlx_offset_param_output_feature_map_182+i];
}
// print __xlx_apatb_param_output_feature_map_183
for (size_t i = 0; i < __xlx_size_param_output_feature_map_183; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_183)[i] = __xlx_output_feature_map_183__tmp_vec[__xlx_offset_param_output_feature_map_183+i];
}
// print __xlx_apatb_param_output_feature_map_184
for (size_t i = 0; i < __xlx_size_param_output_feature_map_184; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_184)[i] = __xlx_output_feature_map_184__tmp_vec[__xlx_offset_param_output_feature_map_184+i];
}
// print __xlx_apatb_param_output_feature_map_185
for (size_t i = 0; i < __xlx_size_param_output_feature_map_185; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_185)[i] = __xlx_output_feature_map_185__tmp_vec[__xlx_offset_param_output_feature_map_185+i];
}
// print __xlx_apatb_param_output_feature_map_186
for (size_t i = 0; i < __xlx_size_param_output_feature_map_186; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_186)[i] = __xlx_output_feature_map_186__tmp_vec[__xlx_offset_param_output_feature_map_186+i];
}
// print __xlx_apatb_param_output_feature_map_187
for (size_t i = 0; i < __xlx_size_param_output_feature_map_187; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_187)[i] = __xlx_output_feature_map_187__tmp_vec[__xlx_offset_param_output_feature_map_187+i];
}
// print __xlx_apatb_param_output_feature_map_188
for (size_t i = 0; i < __xlx_size_param_output_feature_map_188; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_188)[i] = __xlx_output_feature_map_188__tmp_vec[__xlx_offset_param_output_feature_map_188+i];
}
// print __xlx_apatb_param_output_feature_map_189
for (size_t i = 0; i < __xlx_size_param_output_feature_map_189; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_189)[i] = __xlx_output_feature_map_189__tmp_vec[__xlx_offset_param_output_feature_map_189+i];
}
// print __xlx_apatb_param_output_feature_map_190
for (size_t i = 0; i < __xlx_size_param_output_feature_map_190; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_190)[i] = __xlx_output_feature_map_190__tmp_vec[__xlx_offset_param_output_feature_map_190+i];
}
// print __xlx_apatb_param_output_feature_map_191
for (size_t i = 0; i < __xlx_size_param_output_feature_map_191; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_191)[i] = __xlx_output_feature_map_191__tmp_vec[__xlx_offset_param_output_feature_map_191+i];
}
// print __xlx_apatb_param_output_feature_map_192
for (size_t i = 0; i < __xlx_size_param_output_feature_map_192; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_192)[i] = __xlx_output_feature_map_192__tmp_vec[__xlx_offset_param_output_feature_map_192+i];
}
// print __xlx_apatb_param_output_feature_map_193
for (size_t i = 0; i < __xlx_size_param_output_feature_map_193; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_193)[i] = __xlx_output_feature_map_193__tmp_vec[__xlx_offset_param_output_feature_map_193+i];
}
// print __xlx_apatb_param_output_feature_map_194
for (size_t i = 0; i < __xlx_size_param_output_feature_map_194; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_194)[i] = __xlx_output_feature_map_194__tmp_vec[__xlx_offset_param_output_feature_map_194+i];
}
// print __xlx_apatb_param_output_feature_map_195
for (size_t i = 0; i < __xlx_size_param_output_feature_map_195; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_195)[i] = __xlx_output_feature_map_195__tmp_vec[__xlx_offset_param_output_feature_map_195+i];
}
// print __xlx_apatb_param_output_feature_map_196
for (size_t i = 0; i < __xlx_size_param_output_feature_map_196; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_196)[i] = __xlx_output_feature_map_196__tmp_vec[__xlx_offset_param_output_feature_map_196+i];
}
// print __xlx_apatb_param_output_feature_map_197
for (size_t i = 0; i < __xlx_size_param_output_feature_map_197; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_197)[i] = __xlx_output_feature_map_197__tmp_vec[__xlx_offset_param_output_feature_map_197+i];
}
// print __xlx_apatb_param_output_feature_map_198
for (size_t i = 0; i < __xlx_size_param_output_feature_map_198; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_198)[i] = __xlx_output_feature_map_198__tmp_vec[__xlx_offset_param_output_feature_map_198+i];
}
// print __xlx_apatb_param_output_feature_map_199
for (size_t i = 0; i < __xlx_size_param_output_feature_map_199; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_199)[i] = __xlx_output_feature_map_199__tmp_vec[__xlx_offset_param_output_feature_map_199+i];
}
// print __xlx_apatb_param_output_feature_map_200
for (size_t i = 0; i < __xlx_size_param_output_feature_map_200; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_200)[i] = __xlx_output_feature_map_200__tmp_vec[__xlx_offset_param_output_feature_map_200+i];
}
// print __xlx_apatb_param_output_feature_map_201
for (size_t i = 0; i < __xlx_size_param_output_feature_map_201; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_201)[i] = __xlx_output_feature_map_201__tmp_vec[__xlx_offset_param_output_feature_map_201+i];
}
// print __xlx_apatb_param_output_feature_map_202
for (size_t i = 0; i < __xlx_size_param_output_feature_map_202; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_202)[i] = __xlx_output_feature_map_202__tmp_vec[__xlx_offset_param_output_feature_map_202+i];
}
// print __xlx_apatb_param_output_feature_map_203
for (size_t i = 0; i < __xlx_size_param_output_feature_map_203; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_203)[i] = __xlx_output_feature_map_203__tmp_vec[__xlx_offset_param_output_feature_map_203+i];
}
// print __xlx_apatb_param_output_feature_map_204
for (size_t i = 0; i < __xlx_size_param_output_feature_map_204; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_204)[i] = __xlx_output_feature_map_204__tmp_vec[__xlx_offset_param_output_feature_map_204+i];
}
// print __xlx_apatb_param_output_feature_map_205
for (size_t i = 0; i < __xlx_size_param_output_feature_map_205; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_205)[i] = __xlx_output_feature_map_205__tmp_vec[__xlx_offset_param_output_feature_map_205+i];
}
// print __xlx_apatb_param_output_feature_map_206
for (size_t i = 0; i < __xlx_size_param_output_feature_map_206; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_206)[i] = __xlx_output_feature_map_206__tmp_vec[__xlx_offset_param_output_feature_map_206+i];
}
// print __xlx_apatb_param_output_feature_map_207
for (size_t i = 0; i < __xlx_size_param_output_feature_map_207; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_207)[i] = __xlx_output_feature_map_207__tmp_vec[__xlx_offset_param_output_feature_map_207+i];
}
// print __xlx_apatb_param_output_feature_map_208
for (size_t i = 0; i < __xlx_size_param_output_feature_map_208; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_208)[i] = __xlx_output_feature_map_208__tmp_vec[__xlx_offset_param_output_feature_map_208+i];
}
// print __xlx_apatb_param_output_feature_map_209
for (size_t i = 0; i < __xlx_size_param_output_feature_map_209; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_209)[i] = __xlx_output_feature_map_209__tmp_vec[__xlx_offset_param_output_feature_map_209+i];
}
// print __xlx_apatb_param_output_feature_map_210
for (size_t i = 0; i < __xlx_size_param_output_feature_map_210; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_210)[i] = __xlx_output_feature_map_210__tmp_vec[__xlx_offset_param_output_feature_map_210+i];
}
// print __xlx_apatb_param_output_feature_map_211
for (size_t i = 0; i < __xlx_size_param_output_feature_map_211; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_211)[i] = __xlx_output_feature_map_211__tmp_vec[__xlx_offset_param_output_feature_map_211+i];
}
// print __xlx_apatb_param_output_feature_map_212
for (size_t i = 0; i < __xlx_size_param_output_feature_map_212; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_212)[i] = __xlx_output_feature_map_212__tmp_vec[__xlx_offset_param_output_feature_map_212+i];
}
// print __xlx_apatb_param_output_feature_map_213
for (size_t i = 0; i < __xlx_size_param_output_feature_map_213; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_213)[i] = __xlx_output_feature_map_213__tmp_vec[__xlx_offset_param_output_feature_map_213+i];
}
// print __xlx_apatb_param_output_feature_map_214
for (size_t i = 0; i < __xlx_size_param_output_feature_map_214; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_214)[i] = __xlx_output_feature_map_214__tmp_vec[__xlx_offset_param_output_feature_map_214+i];
}
// print __xlx_apatb_param_output_feature_map_215
for (size_t i = 0; i < __xlx_size_param_output_feature_map_215; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_215)[i] = __xlx_output_feature_map_215__tmp_vec[__xlx_offset_param_output_feature_map_215+i];
}
// print __xlx_apatb_param_output_feature_map_216
for (size_t i = 0; i < __xlx_size_param_output_feature_map_216; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_216)[i] = __xlx_output_feature_map_216__tmp_vec[__xlx_offset_param_output_feature_map_216+i];
}
// print __xlx_apatb_param_output_feature_map_217
for (size_t i = 0; i < __xlx_size_param_output_feature_map_217; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_217)[i] = __xlx_output_feature_map_217__tmp_vec[__xlx_offset_param_output_feature_map_217+i];
}
// print __xlx_apatb_param_output_feature_map_218
for (size_t i = 0; i < __xlx_size_param_output_feature_map_218; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_218)[i] = __xlx_output_feature_map_218__tmp_vec[__xlx_offset_param_output_feature_map_218+i];
}
// print __xlx_apatb_param_output_feature_map_219
for (size_t i = 0; i < __xlx_size_param_output_feature_map_219; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_219)[i] = __xlx_output_feature_map_219__tmp_vec[__xlx_offset_param_output_feature_map_219+i];
}
// print __xlx_apatb_param_output_feature_map_220
for (size_t i = 0; i < __xlx_size_param_output_feature_map_220; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_220)[i] = __xlx_output_feature_map_220__tmp_vec[__xlx_offset_param_output_feature_map_220+i];
}
// print __xlx_apatb_param_output_feature_map_221
for (size_t i = 0; i < __xlx_size_param_output_feature_map_221; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_221)[i] = __xlx_output_feature_map_221__tmp_vec[__xlx_offset_param_output_feature_map_221+i];
}
// print __xlx_apatb_param_output_feature_map_222
for (size_t i = 0; i < __xlx_size_param_output_feature_map_222; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_222)[i] = __xlx_output_feature_map_222__tmp_vec[__xlx_offset_param_output_feature_map_222+i];
}
// print __xlx_apatb_param_output_feature_map_223
for (size_t i = 0; i < __xlx_size_param_output_feature_map_223; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_223)[i] = __xlx_output_feature_map_223__tmp_vec[__xlx_offset_param_output_feature_map_223+i];
}
// print __xlx_apatb_param_output_feature_map_224
for (size_t i = 0; i < __xlx_size_param_output_feature_map_224; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_224)[i] = __xlx_output_feature_map_224__tmp_vec[__xlx_offset_param_output_feature_map_224+i];
}
// print __xlx_apatb_param_output_feature_map_225
for (size_t i = 0; i < __xlx_size_param_output_feature_map_225; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_225)[i] = __xlx_output_feature_map_225__tmp_vec[__xlx_offset_param_output_feature_map_225+i];
}
// print __xlx_apatb_param_output_feature_map_226
for (size_t i = 0; i < __xlx_size_param_output_feature_map_226; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_226)[i] = __xlx_output_feature_map_226__tmp_vec[__xlx_offset_param_output_feature_map_226+i];
}
// print __xlx_apatb_param_output_feature_map_227
for (size_t i = 0; i < __xlx_size_param_output_feature_map_227; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_227)[i] = __xlx_output_feature_map_227__tmp_vec[__xlx_offset_param_output_feature_map_227+i];
}
// print __xlx_apatb_param_output_feature_map_228
for (size_t i = 0; i < __xlx_size_param_output_feature_map_228; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_228)[i] = __xlx_output_feature_map_228__tmp_vec[__xlx_offset_param_output_feature_map_228+i];
}
// print __xlx_apatb_param_output_feature_map_229
for (size_t i = 0; i < __xlx_size_param_output_feature_map_229; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_229)[i] = __xlx_output_feature_map_229__tmp_vec[__xlx_offset_param_output_feature_map_229+i];
}
// print __xlx_apatb_param_output_feature_map_230
for (size_t i = 0; i < __xlx_size_param_output_feature_map_230; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_230)[i] = __xlx_output_feature_map_230__tmp_vec[__xlx_offset_param_output_feature_map_230+i];
}
// print __xlx_apatb_param_output_feature_map_231
for (size_t i = 0; i < __xlx_size_param_output_feature_map_231; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_231)[i] = __xlx_output_feature_map_231__tmp_vec[__xlx_offset_param_output_feature_map_231+i];
}
// print __xlx_apatb_param_output_feature_map_232
for (size_t i = 0; i < __xlx_size_param_output_feature_map_232; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_232)[i] = __xlx_output_feature_map_232__tmp_vec[__xlx_offset_param_output_feature_map_232+i];
}
// print __xlx_apatb_param_output_feature_map_233
for (size_t i = 0; i < __xlx_size_param_output_feature_map_233; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_233)[i] = __xlx_output_feature_map_233__tmp_vec[__xlx_offset_param_output_feature_map_233+i];
}
// print __xlx_apatb_param_output_feature_map_234
for (size_t i = 0; i < __xlx_size_param_output_feature_map_234; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_234)[i] = __xlx_output_feature_map_234__tmp_vec[__xlx_offset_param_output_feature_map_234+i];
}
// print __xlx_apatb_param_output_feature_map_235
for (size_t i = 0; i < __xlx_size_param_output_feature_map_235; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_235)[i] = __xlx_output_feature_map_235__tmp_vec[__xlx_offset_param_output_feature_map_235+i];
}
// print __xlx_apatb_param_output_feature_map_236
for (size_t i = 0; i < __xlx_size_param_output_feature_map_236; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_236)[i] = __xlx_output_feature_map_236__tmp_vec[__xlx_offset_param_output_feature_map_236+i];
}
// print __xlx_apatb_param_output_feature_map_237
for (size_t i = 0; i < __xlx_size_param_output_feature_map_237; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_237)[i] = __xlx_output_feature_map_237__tmp_vec[__xlx_offset_param_output_feature_map_237+i];
}
// print __xlx_apatb_param_output_feature_map_238
for (size_t i = 0; i < __xlx_size_param_output_feature_map_238; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_238)[i] = __xlx_output_feature_map_238__tmp_vec[__xlx_offset_param_output_feature_map_238+i];
}
// print __xlx_apatb_param_output_feature_map_239
for (size_t i = 0; i < __xlx_size_param_output_feature_map_239; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_239)[i] = __xlx_output_feature_map_239__tmp_vec[__xlx_offset_param_output_feature_map_239+i];
}
// print __xlx_apatb_param_output_feature_map_240
for (size_t i = 0; i < __xlx_size_param_output_feature_map_240; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_240)[i] = __xlx_output_feature_map_240__tmp_vec[__xlx_offset_param_output_feature_map_240+i];
}
// print __xlx_apatb_param_output_feature_map_241
for (size_t i = 0; i < __xlx_size_param_output_feature_map_241; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_241)[i] = __xlx_output_feature_map_241__tmp_vec[__xlx_offset_param_output_feature_map_241+i];
}
// print __xlx_apatb_param_output_feature_map_242
for (size_t i = 0; i < __xlx_size_param_output_feature_map_242; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_242)[i] = __xlx_output_feature_map_242__tmp_vec[__xlx_offset_param_output_feature_map_242+i];
}
// print __xlx_apatb_param_output_feature_map_243
for (size_t i = 0; i < __xlx_size_param_output_feature_map_243; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_243)[i] = __xlx_output_feature_map_243__tmp_vec[__xlx_offset_param_output_feature_map_243+i];
}
// print __xlx_apatb_param_output_feature_map_244
for (size_t i = 0; i < __xlx_size_param_output_feature_map_244; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_244)[i] = __xlx_output_feature_map_244__tmp_vec[__xlx_offset_param_output_feature_map_244+i];
}
// print __xlx_apatb_param_output_feature_map_245
for (size_t i = 0; i < __xlx_size_param_output_feature_map_245; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_245)[i] = __xlx_output_feature_map_245__tmp_vec[__xlx_offset_param_output_feature_map_245+i];
}
// print __xlx_apatb_param_output_feature_map_246
for (size_t i = 0; i < __xlx_size_param_output_feature_map_246; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_246)[i] = __xlx_output_feature_map_246__tmp_vec[__xlx_offset_param_output_feature_map_246+i];
}
// print __xlx_apatb_param_output_feature_map_247
for (size_t i = 0; i < __xlx_size_param_output_feature_map_247; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_247)[i] = __xlx_output_feature_map_247__tmp_vec[__xlx_offset_param_output_feature_map_247+i];
}
// print __xlx_apatb_param_output_feature_map_248
for (size_t i = 0; i < __xlx_size_param_output_feature_map_248; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_248)[i] = __xlx_output_feature_map_248__tmp_vec[__xlx_offset_param_output_feature_map_248+i];
}
// print __xlx_apatb_param_output_feature_map_249
for (size_t i = 0; i < __xlx_size_param_output_feature_map_249; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_249)[i] = __xlx_output_feature_map_249__tmp_vec[__xlx_offset_param_output_feature_map_249+i];
}
// print __xlx_apatb_param_output_feature_map_250
for (size_t i = 0; i < __xlx_size_param_output_feature_map_250; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_250)[i] = __xlx_output_feature_map_250__tmp_vec[__xlx_offset_param_output_feature_map_250+i];
}
// print __xlx_apatb_param_output_feature_map_251
for (size_t i = 0; i < __xlx_size_param_output_feature_map_251; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_251)[i] = __xlx_output_feature_map_251__tmp_vec[__xlx_offset_param_output_feature_map_251+i];
}
// print __xlx_apatb_param_output_feature_map_252
for (size_t i = 0; i < __xlx_size_param_output_feature_map_252; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_252)[i] = __xlx_output_feature_map_252__tmp_vec[__xlx_offset_param_output_feature_map_252+i];
}
// print __xlx_apatb_param_output_feature_map_253
for (size_t i = 0; i < __xlx_size_param_output_feature_map_253; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_253)[i] = __xlx_output_feature_map_253__tmp_vec[__xlx_offset_param_output_feature_map_253+i];
}
// print __xlx_apatb_param_output_feature_map_254
for (size_t i = 0; i < __xlx_size_param_output_feature_map_254; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_254)[i] = __xlx_output_feature_map_254__tmp_vec[__xlx_offset_param_output_feature_map_254+i];
}
// print __xlx_apatb_param_output_feature_map_255
for (size_t i = 0; i < __xlx_size_param_output_feature_map_255; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_255)[i] = __xlx_output_feature_map_255__tmp_vec[__xlx_offset_param_output_feature_map_255+i];
}
// print __xlx_apatb_param_output_feature_map_256
for (size_t i = 0; i < __xlx_size_param_output_feature_map_256; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_256)[i] = __xlx_output_feature_map_256__tmp_vec[__xlx_offset_param_output_feature_map_256+i];
}
// print __xlx_apatb_param_output_feature_map_257
for (size_t i = 0; i < __xlx_size_param_output_feature_map_257; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_257)[i] = __xlx_output_feature_map_257__tmp_vec[__xlx_offset_param_output_feature_map_257+i];
}
// print __xlx_apatb_param_output_feature_map_258
for (size_t i = 0; i < __xlx_size_param_output_feature_map_258; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_258)[i] = __xlx_output_feature_map_258__tmp_vec[__xlx_offset_param_output_feature_map_258+i];
}
// print __xlx_apatb_param_output_feature_map_259
for (size_t i = 0; i < __xlx_size_param_output_feature_map_259; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_259)[i] = __xlx_output_feature_map_259__tmp_vec[__xlx_offset_param_output_feature_map_259+i];
}
// print __xlx_apatb_param_output_feature_map_260
for (size_t i = 0; i < __xlx_size_param_output_feature_map_260; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_260)[i] = __xlx_output_feature_map_260__tmp_vec[__xlx_offset_param_output_feature_map_260+i];
}
// print __xlx_apatb_param_output_feature_map_261
for (size_t i = 0; i < __xlx_size_param_output_feature_map_261; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_261)[i] = __xlx_output_feature_map_261__tmp_vec[__xlx_offset_param_output_feature_map_261+i];
}
// print __xlx_apatb_param_output_feature_map_262
for (size_t i = 0; i < __xlx_size_param_output_feature_map_262; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_262)[i] = __xlx_output_feature_map_262__tmp_vec[__xlx_offset_param_output_feature_map_262+i];
}
// print __xlx_apatb_param_output_feature_map_263
for (size_t i = 0; i < __xlx_size_param_output_feature_map_263; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_263)[i] = __xlx_output_feature_map_263__tmp_vec[__xlx_offset_param_output_feature_map_263+i];
}
// print __xlx_apatb_param_output_feature_map_264
for (size_t i = 0; i < __xlx_size_param_output_feature_map_264; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_264)[i] = __xlx_output_feature_map_264__tmp_vec[__xlx_offset_param_output_feature_map_264+i];
}
// print __xlx_apatb_param_output_feature_map_265
for (size_t i = 0; i < __xlx_size_param_output_feature_map_265; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_265)[i] = __xlx_output_feature_map_265__tmp_vec[__xlx_offset_param_output_feature_map_265+i];
}
// print __xlx_apatb_param_output_feature_map_266
for (size_t i = 0; i < __xlx_size_param_output_feature_map_266; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_266)[i] = __xlx_output_feature_map_266__tmp_vec[__xlx_offset_param_output_feature_map_266+i];
}
// print __xlx_apatb_param_output_feature_map_267
for (size_t i = 0; i < __xlx_size_param_output_feature_map_267; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_267)[i] = __xlx_output_feature_map_267__tmp_vec[__xlx_offset_param_output_feature_map_267+i];
}
// print __xlx_apatb_param_output_feature_map_268
for (size_t i = 0; i < __xlx_size_param_output_feature_map_268; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_268)[i] = __xlx_output_feature_map_268__tmp_vec[__xlx_offset_param_output_feature_map_268+i];
}
// print __xlx_apatb_param_output_feature_map_269
for (size_t i = 0; i < __xlx_size_param_output_feature_map_269; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_269)[i] = __xlx_output_feature_map_269__tmp_vec[__xlx_offset_param_output_feature_map_269+i];
}
// print __xlx_apatb_param_output_feature_map_270
for (size_t i = 0; i < __xlx_size_param_output_feature_map_270; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_270)[i] = __xlx_output_feature_map_270__tmp_vec[__xlx_offset_param_output_feature_map_270+i];
}
// print __xlx_apatb_param_output_feature_map_271
for (size_t i = 0; i < __xlx_size_param_output_feature_map_271; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_271)[i] = __xlx_output_feature_map_271__tmp_vec[__xlx_offset_param_output_feature_map_271+i];
}
// print __xlx_apatb_param_output_feature_map_272
for (size_t i = 0; i < __xlx_size_param_output_feature_map_272; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_272)[i] = __xlx_output_feature_map_272__tmp_vec[__xlx_offset_param_output_feature_map_272+i];
}
// print __xlx_apatb_param_output_feature_map_273
for (size_t i = 0; i < __xlx_size_param_output_feature_map_273; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_273)[i] = __xlx_output_feature_map_273__tmp_vec[__xlx_offset_param_output_feature_map_273+i];
}
// print __xlx_apatb_param_output_feature_map_274
for (size_t i = 0; i < __xlx_size_param_output_feature_map_274; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_274)[i] = __xlx_output_feature_map_274__tmp_vec[__xlx_offset_param_output_feature_map_274+i];
}
// print __xlx_apatb_param_output_feature_map_275
for (size_t i = 0; i < __xlx_size_param_output_feature_map_275; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_275)[i] = __xlx_output_feature_map_275__tmp_vec[__xlx_offset_param_output_feature_map_275+i];
}
// print __xlx_apatb_param_output_feature_map_276
for (size_t i = 0; i < __xlx_size_param_output_feature_map_276; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_276)[i] = __xlx_output_feature_map_276__tmp_vec[__xlx_offset_param_output_feature_map_276+i];
}
// print __xlx_apatb_param_output_feature_map_277
for (size_t i = 0; i < __xlx_size_param_output_feature_map_277; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_277)[i] = __xlx_output_feature_map_277__tmp_vec[__xlx_offset_param_output_feature_map_277+i];
}
// print __xlx_apatb_param_output_feature_map_278
for (size_t i = 0; i < __xlx_size_param_output_feature_map_278; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_278)[i] = __xlx_output_feature_map_278__tmp_vec[__xlx_offset_param_output_feature_map_278+i];
}
// print __xlx_apatb_param_output_feature_map_279
for (size_t i = 0; i < __xlx_size_param_output_feature_map_279; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_279)[i] = __xlx_output_feature_map_279__tmp_vec[__xlx_offset_param_output_feature_map_279+i];
}
// print __xlx_apatb_param_output_feature_map_280
for (size_t i = 0; i < __xlx_size_param_output_feature_map_280; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_280)[i] = __xlx_output_feature_map_280__tmp_vec[__xlx_offset_param_output_feature_map_280+i];
}
// print __xlx_apatb_param_output_feature_map_281
for (size_t i = 0; i < __xlx_size_param_output_feature_map_281; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_281)[i] = __xlx_output_feature_map_281__tmp_vec[__xlx_offset_param_output_feature_map_281+i];
}
// print __xlx_apatb_param_output_feature_map_282
for (size_t i = 0; i < __xlx_size_param_output_feature_map_282; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_282)[i] = __xlx_output_feature_map_282__tmp_vec[__xlx_offset_param_output_feature_map_282+i];
}
// print __xlx_apatb_param_output_feature_map_283
for (size_t i = 0; i < __xlx_size_param_output_feature_map_283; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_283)[i] = __xlx_output_feature_map_283__tmp_vec[__xlx_offset_param_output_feature_map_283+i];
}
// print __xlx_apatb_param_output_feature_map_284
for (size_t i = 0; i < __xlx_size_param_output_feature_map_284; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_284)[i] = __xlx_output_feature_map_284__tmp_vec[__xlx_offset_param_output_feature_map_284+i];
}
// print __xlx_apatb_param_output_feature_map_285
for (size_t i = 0; i < __xlx_size_param_output_feature_map_285; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_285)[i] = __xlx_output_feature_map_285__tmp_vec[__xlx_offset_param_output_feature_map_285+i];
}
// print __xlx_apatb_param_output_feature_map_286
for (size_t i = 0; i < __xlx_size_param_output_feature_map_286; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_286)[i] = __xlx_output_feature_map_286__tmp_vec[__xlx_offset_param_output_feature_map_286+i];
}
// print __xlx_apatb_param_output_feature_map_287
for (size_t i = 0; i < __xlx_size_param_output_feature_map_287; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_287)[i] = __xlx_output_feature_map_287__tmp_vec[__xlx_offset_param_output_feature_map_287+i];
}
// print __xlx_apatb_param_output_feature_map_288
for (size_t i = 0; i < __xlx_size_param_output_feature_map_288; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_288)[i] = __xlx_output_feature_map_288__tmp_vec[__xlx_offset_param_output_feature_map_288+i];
}
// print __xlx_apatb_param_output_feature_map_289
for (size_t i = 0; i < __xlx_size_param_output_feature_map_289; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_289)[i] = __xlx_output_feature_map_289__tmp_vec[__xlx_offset_param_output_feature_map_289+i];
}
// print __xlx_apatb_param_output_feature_map_290
for (size_t i = 0; i < __xlx_size_param_output_feature_map_290; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_290)[i] = __xlx_output_feature_map_290__tmp_vec[__xlx_offset_param_output_feature_map_290+i];
}
// print __xlx_apatb_param_output_feature_map_291
for (size_t i = 0; i < __xlx_size_param_output_feature_map_291; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_291)[i] = __xlx_output_feature_map_291__tmp_vec[__xlx_offset_param_output_feature_map_291+i];
}
// print __xlx_apatb_param_output_feature_map_292
for (size_t i = 0; i < __xlx_size_param_output_feature_map_292; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_292)[i] = __xlx_output_feature_map_292__tmp_vec[__xlx_offset_param_output_feature_map_292+i];
}
// print __xlx_apatb_param_output_feature_map_293
for (size_t i = 0; i < __xlx_size_param_output_feature_map_293; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_293)[i] = __xlx_output_feature_map_293__tmp_vec[__xlx_offset_param_output_feature_map_293+i];
}
// print __xlx_apatb_param_output_feature_map_294
for (size_t i = 0; i < __xlx_size_param_output_feature_map_294; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_294)[i] = __xlx_output_feature_map_294__tmp_vec[__xlx_offset_param_output_feature_map_294+i];
}
// print __xlx_apatb_param_output_feature_map_295
for (size_t i = 0; i < __xlx_size_param_output_feature_map_295; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_295)[i] = __xlx_output_feature_map_295__tmp_vec[__xlx_offset_param_output_feature_map_295+i];
}
// print __xlx_apatb_param_output_feature_map_296
for (size_t i = 0; i < __xlx_size_param_output_feature_map_296; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_296)[i] = __xlx_output_feature_map_296__tmp_vec[__xlx_offset_param_output_feature_map_296+i];
}
// print __xlx_apatb_param_output_feature_map_297
for (size_t i = 0; i < __xlx_size_param_output_feature_map_297; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_297)[i] = __xlx_output_feature_map_297__tmp_vec[__xlx_offset_param_output_feature_map_297+i];
}
// print __xlx_apatb_param_output_feature_map_298
for (size_t i = 0; i < __xlx_size_param_output_feature_map_298; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_298)[i] = __xlx_output_feature_map_298__tmp_vec[__xlx_offset_param_output_feature_map_298+i];
}
// print __xlx_apatb_param_output_feature_map_299
for (size_t i = 0; i < __xlx_size_param_output_feature_map_299; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_299)[i] = __xlx_output_feature_map_299__tmp_vec[__xlx_offset_param_output_feature_map_299+i];
}
// print __xlx_apatb_param_output_feature_map_300
for (size_t i = 0; i < __xlx_size_param_output_feature_map_300; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_300)[i] = __xlx_output_feature_map_300__tmp_vec[__xlx_offset_param_output_feature_map_300+i];
}
// print __xlx_apatb_param_output_feature_map_301
for (size_t i = 0; i < __xlx_size_param_output_feature_map_301; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_301)[i] = __xlx_output_feature_map_301__tmp_vec[__xlx_offset_param_output_feature_map_301+i];
}
// print __xlx_apatb_param_output_feature_map_302
for (size_t i = 0; i < __xlx_size_param_output_feature_map_302; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_302)[i] = __xlx_output_feature_map_302__tmp_vec[__xlx_offset_param_output_feature_map_302+i];
}
// print __xlx_apatb_param_output_feature_map_303
for (size_t i = 0; i < __xlx_size_param_output_feature_map_303; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_303)[i] = __xlx_output_feature_map_303__tmp_vec[__xlx_offset_param_output_feature_map_303+i];
}
// print __xlx_apatb_param_output_feature_map_304
for (size_t i = 0; i < __xlx_size_param_output_feature_map_304; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_304)[i] = __xlx_output_feature_map_304__tmp_vec[__xlx_offset_param_output_feature_map_304+i];
}
// print __xlx_apatb_param_output_feature_map_305
for (size_t i = 0; i < __xlx_size_param_output_feature_map_305; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_305)[i] = __xlx_output_feature_map_305__tmp_vec[__xlx_offset_param_output_feature_map_305+i];
}
// print __xlx_apatb_param_output_feature_map_306
for (size_t i = 0; i < __xlx_size_param_output_feature_map_306; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_306)[i] = __xlx_output_feature_map_306__tmp_vec[__xlx_offset_param_output_feature_map_306+i];
}
// print __xlx_apatb_param_output_feature_map_307
for (size_t i = 0; i < __xlx_size_param_output_feature_map_307; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_307)[i] = __xlx_output_feature_map_307__tmp_vec[__xlx_offset_param_output_feature_map_307+i];
}
// print __xlx_apatb_param_output_feature_map_308
for (size_t i = 0; i < __xlx_size_param_output_feature_map_308; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_308)[i] = __xlx_output_feature_map_308__tmp_vec[__xlx_offset_param_output_feature_map_308+i];
}
// print __xlx_apatb_param_output_feature_map_309
for (size_t i = 0; i < __xlx_size_param_output_feature_map_309; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_309)[i] = __xlx_output_feature_map_309__tmp_vec[__xlx_offset_param_output_feature_map_309+i];
}
// print __xlx_apatb_param_output_feature_map_310
for (size_t i = 0; i < __xlx_size_param_output_feature_map_310; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_310)[i] = __xlx_output_feature_map_310__tmp_vec[__xlx_offset_param_output_feature_map_310+i];
}
// print __xlx_apatb_param_output_feature_map_311
for (size_t i = 0; i < __xlx_size_param_output_feature_map_311; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_311)[i] = __xlx_output_feature_map_311__tmp_vec[__xlx_offset_param_output_feature_map_311+i];
}
// print __xlx_apatb_param_output_feature_map_312
for (size_t i = 0; i < __xlx_size_param_output_feature_map_312; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_312)[i] = __xlx_output_feature_map_312__tmp_vec[__xlx_offset_param_output_feature_map_312+i];
}
// print __xlx_apatb_param_output_feature_map_313
for (size_t i = 0; i < __xlx_size_param_output_feature_map_313; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_313)[i] = __xlx_output_feature_map_313__tmp_vec[__xlx_offset_param_output_feature_map_313+i];
}
// print __xlx_apatb_param_output_feature_map_314
for (size_t i = 0; i < __xlx_size_param_output_feature_map_314; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_314)[i] = __xlx_output_feature_map_314__tmp_vec[__xlx_offset_param_output_feature_map_314+i];
}
// print __xlx_apatb_param_output_feature_map_315
for (size_t i = 0; i < __xlx_size_param_output_feature_map_315; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_315)[i] = __xlx_output_feature_map_315__tmp_vec[__xlx_offset_param_output_feature_map_315+i];
}
// print __xlx_apatb_param_output_feature_map_316
for (size_t i = 0; i < __xlx_size_param_output_feature_map_316; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_316)[i] = __xlx_output_feature_map_316__tmp_vec[__xlx_offset_param_output_feature_map_316+i];
}
// print __xlx_apatb_param_output_feature_map_317
for (size_t i = 0; i < __xlx_size_param_output_feature_map_317; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_317)[i] = __xlx_output_feature_map_317__tmp_vec[__xlx_offset_param_output_feature_map_317+i];
}
// print __xlx_apatb_param_output_feature_map_318
for (size_t i = 0; i < __xlx_size_param_output_feature_map_318; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_318)[i] = __xlx_output_feature_map_318__tmp_vec[__xlx_offset_param_output_feature_map_318+i];
}
// print __xlx_apatb_param_output_feature_map_319
for (size_t i = 0; i < __xlx_size_param_output_feature_map_319; ++i) {
((Byte<2>*)__xlx_apatb_param_output_feature_map_319)[i] = __xlx_output_feature_map_319__tmp_vec[__xlx_offset_param_output_feature_map_319+i];
}
}
