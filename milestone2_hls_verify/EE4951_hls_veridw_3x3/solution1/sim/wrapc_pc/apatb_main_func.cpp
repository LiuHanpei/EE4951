#include "hls_signal_handler.h"
#include <algorithm>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include "ap_fixed.h"
#include "ap_int.h"
#include "autopilot_cbe.h"
#include "hls_half.h"
#include "hls_stream.h"

using namespace std;

// wrapc file define:
#define AUTOTB_TVIN_input_feature_map "../tv/cdatafile/c.main_func.autotvin_input_feature_map.dat"
#define AUTOTB_TVOUT_input_feature_map "../tv/cdatafile/c.main_func.autotvout_input_feature_map.dat"
#define AUTOTB_TVIN_layer_weights "../tv/cdatafile/c.main_func.autotvin_layer_weights.dat"
#define AUTOTB_TVOUT_layer_weights "../tv/cdatafile/c.main_func.autotvout_layer_weights.dat"
#define AUTOTB_TVIN_layer_bias "../tv/cdatafile/c.main_func.autotvin_layer_bias.dat"
#define AUTOTB_TVOUT_layer_bias "../tv/cdatafile/c.main_func.autotvout_layer_bias.dat"
#define AUTOTB_TVIN_output_feature_map_0 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_0.dat"
#define AUTOTB_TVOUT_output_feature_map_0 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_0.dat"
#define AUTOTB_TVIN_output_feature_map_1 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_1.dat"
#define AUTOTB_TVOUT_output_feature_map_1 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_1.dat"
#define AUTOTB_TVIN_output_feature_map_2 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_2.dat"
#define AUTOTB_TVOUT_output_feature_map_2 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_2.dat"
#define AUTOTB_TVIN_output_feature_map_3 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_3.dat"
#define AUTOTB_TVOUT_output_feature_map_3 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_3.dat"
#define AUTOTB_TVIN_output_feature_map_4 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_4.dat"
#define AUTOTB_TVOUT_output_feature_map_4 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_4.dat"
#define AUTOTB_TVIN_output_feature_map_5 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_5.dat"
#define AUTOTB_TVOUT_output_feature_map_5 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_5.dat"
#define AUTOTB_TVIN_output_feature_map_6 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_6.dat"
#define AUTOTB_TVOUT_output_feature_map_6 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_6.dat"
#define AUTOTB_TVIN_output_feature_map_7 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_7.dat"
#define AUTOTB_TVOUT_output_feature_map_7 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_7.dat"
#define AUTOTB_TVIN_output_feature_map_8 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_8.dat"
#define AUTOTB_TVOUT_output_feature_map_8 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_8.dat"
#define AUTOTB_TVIN_output_feature_map_9 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_9.dat"
#define AUTOTB_TVOUT_output_feature_map_9 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_9.dat"
#define AUTOTB_TVIN_output_feature_map_10 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_10.dat"
#define AUTOTB_TVOUT_output_feature_map_10 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_10.dat"
#define AUTOTB_TVIN_output_feature_map_11 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_11.dat"
#define AUTOTB_TVOUT_output_feature_map_11 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_11.dat"
#define AUTOTB_TVIN_output_feature_map_12 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_12.dat"
#define AUTOTB_TVOUT_output_feature_map_12 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_12.dat"
#define AUTOTB_TVIN_output_feature_map_13 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_13.dat"
#define AUTOTB_TVOUT_output_feature_map_13 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_13.dat"
#define AUTOTB_TVIN_output_feature_map_14 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_14.dat"
#define AUTOTB_TVOUT_output_feature_map_14 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_14.dat"
#define AUTOTB_TVIN_output_feature_map_15 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_15.dat"
#define AUTOTB_TVOUT_output_feature_map_15 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_15.dat"
#define AUTOTB_TVIN_output_feature_map_16 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_16.dat"
#define AUTOTB_TVOUT_output_feature_map_16 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_16.dat"
#define AUTOTB_TVIN_output_feature_map_17 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_17.dat"
#define AUTOTB_TVOUT_output_feature_map_17 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_17.dat"
#define AUTOTB_TVIN_output_feature_map_18 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_18.dat"
#define AUTOTB_TVOUT_output_feature_map_18 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_18.dat"
#define AUTOTB_TVIN_output_feature_map_19 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_19.dat"
#define AUTOTB_TVOUT_output_feature_map_19 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_19.dat"
#define AUTOTB_TVIN_output_feature_map_20 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_20.dat"
#define AUTOTB_TVOUT_output_feature_map_20 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_20.dat"
#define AUTOTB_TVIN_output_feature_map_21 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_21.dat"
#define AUTOTB_TVOUT_output_feature_map_21 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_21.dat"
#define AUTOTB_TVIN_output_feature_map_22 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_22.dat"
#define AUTOTB_TVOUT_output_feature_map_22 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_22.dat"
#define AUTOTB_TVIN_output_feature_map_23 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_23.dat"
#define AUTOTB_TVOUT_output_feature_map_23 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_23.dat"
#define AUTOTB_TVIN_output_feature_map_24 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_24.dat"
#define AUTOTB_TVOUT_output_feature_map_24 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_24.dat"
#define AUTOTB_TVIN_output_feature_map_25 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_25.dat"
#define AUTOTB_TVOUT_output_feature_map_25 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_25.dat"
#define AUTOTB_TVIN_output_feature_map_26 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_26.dat"
#define AUTOTB_TVOUT_output_feature_map_26 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_26.dat"
#define AUTOTB_TVIN_output_feature_map_27 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_27.dat"
#define AUTOTB_TVOUT_output_feature_map_27 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_27.dat"
#define AUTOTB_TVIN_output_feature_map_28 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_28.dat"
#define AUTOTB_TVOUT_output_feature_map_28 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_28.dat"
#define AUTOTB_TVIN_output_feature_map_29 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_29.dat"
#define AUTOTB_TVOUT_output_feature_map_29 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_29.dat"
#define AUTOTB_TVIN_output_feature_map_30 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_30.dat"
#define AUTOTB_TVOUT_output_feature_map_30 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_30.dat"
#define AUTOTB_TVIN_output_feature_map_31 "../tv/cdatafile/c.main_func.autotvin_output_feature_map_31.dat"
#define AUTOTB_TVOUT_output_feature_map_31 "../tv/cdatafile/c.main_func.autotvout_output_feature_map_31.dat"
#define AUTOTB_TVIN_fm "../tv/cdatafile/c.main_func.autotvin_fm.dat"
#define AUTOTB_TVOUT_fm "../tv/cdatafile/c.main_func.autotvout_fm.dat"
#define AUTOTB_TVIN_fm_0 "../tv/cdatafile/c.main_func.autotvin_fm_0.dat"
#define AUTOTB_TVOUT_fm_0 "../tv/cdatafile/c.main_func.autotvout_fm_0.dat"
#define AUTOTB_TVIN_fm_1 "../tv/cdatafile/c.main_func.autotvin_fm_1.dat"
#define AUTOTB_TVOUT_fm_1 "../tv/cdatafile/c.main_func.autotvout_fm_1.dat"
#define AUTOTB_TVIN_fm_10 "../tv/cdatafile/c.main_func.autotvin_fm_10.dat"
#define AUTOTB_TVOUT_fm_10 "../tv/cdatafile/c.main_func.autotvout_fm_10.dat"
#define AUTOTB_TVIN_fm_11 "../tv/cdatafile/c.main_func.autotvin_fm_11.dat"
#define AUTOTB_TVOUT_fm_11 "../tv/cdatafile/c.main_func.autotvout_fm_11.dat"
#define AUTOTB_TVIN_fm_12 "../tv/cdatafile/c.main_func.autotvin_fm_12.dat"
#define AUTOTB_TVOUT_fm_12 "../tv/cdatafile/c.main_func.autotvout_fm_12.dat"
#define AUTOTB_TVIN_fm_13 "../tv/cdatafile/c.main_func.autotvin_fm_13.dat"
#define AUTOTB_TVOUT_fm_13 "../tv/cdatafile/c.main_func.autotvout_fm_13.dat"
#define AUTOTB_TVIN_fm_14 "../tv/cdatafile/c.main_func.autotvin_fm_14.dat"
#define AUTOTB_TVOUT_fm_14 "../tv/cdatafile/c.main_func.autotvout_fm_14.dat"
#define AUTOTB_TVIN_fm_15 "../tv/cdatafile/c.main_func.autotvin_fm_15.dat"
#define AUTOTB_TVOUT_fm_15 "../tv/cdatafile/c.main_func.autotvout_fm_15.dat"
#define AUTOTB_TVIN_fm_16 "../tv/cdatafile/c.main_func.autotvin_fm_16.dat"
#define AUTOTB_TVOUT_fm_16 "../tv/cdatafile/c.main_func.autotvout_fm_16.dat"
#define AUTOTB_TVIN_fm_17 "../tv/cdatafile/c.main_func.autotvin_fm_17.dat"
#define AUTOTB_TVOUT_fm_17 "../tv/cdatafile/c.main_func.autotvout_fm_17.dat"
#define AUTOTB_TVIN_fm_18 "../tv/cdatafile/c.main_func.autotvin_fm_18.dat"
#define AUTOTB_TVOUT_fm_18 "../tv/cdatafile/c.main_func.autotvout_fm_18.dat"
#define AUTOTB_TVIN_fm_19 "../tv/cdatafile/c.main_func.autotvin_fm_19.dat"
#define AUTOTB_TVOUT_fm_19 "../tv/cdatafile/c.main_func.autotvout_fm_19.dat"
#define AUTOTB_TVIN_fm_2 "../tv/cdatafile/c.main_func.autotvin_fm_2.dat"
#define AUTOTB_TVOUT_fm_2 "../tv/cdatafile/c.main_func.autotvout_fm_2.dat"
#define AUTOTB_TVIN_fm_20 "../tv/cdatafile/c.main_func.autotvin_fm_20.dat"
#define AUTOTB_TVOUT_fm_20 "../tv/cdatafile/c.main_func.autotvout_fm_20.dat"
#define AUTOTB_TVIN_fm_21 "../tv/cdatafile/c.main_func.autotvin_fm_21.dat"
#define AUTOTB_TVOUT_fm_21 "../tv/cdatafile/c.main_func.autotvout_fm_21.dat"
#define AUTOTB_TVIN_fm_22 "../tv/cdatafile/c.main_func.autotvin_fm_22.dat"
#define AUTOTB_TVOUT_fm_22 "../tv/cdatafile/c.main_func.autotvout_fm_22.dat"
#define AUTOTB_TVIN_fm_23 "../tv/cdatafile/c.main_func.autotvin_fm_23.dat"
#define AUTOTB_TVOUT_fm_23 "../tv/cdatafile/c.main_func.autotvout_fm_23.dat"
#define AUTOTB_TVIN_fm_24 "../tv/cdatafile/c.main_func.autotvin_fm_24.dat"
#define AUTOTB_TVOUT_fm_24 "../tv/cdatafile/c.main_func.autotvout_fm_24.dat"
#define AUTOTB_TVIN_fm_25 "../tv/cdatafile/c.main_func.autotvin_fm_25.dat"
#define AUTOTB_TVOUT_fm_25 "../tv/cdatafile/c.main_func.autotvout_fm_25.dat"
#define AUTOTB_TVIN_fm_26 "../tv/cdatafile/c.main_func.autotvin_fm_26.dat"
#define AUTOTB_TVOUT_fm_26 "../tv/cdatafile/c.main_func.autotvout_fm_26.dat"
#define AUTOTB_TVIN_fm_27 "../tv/cdatafile/c.main_func.autotvin_fm_27.dat"
#define AUTOTB_TVOUT_fm_27 "../tv/cdatafile/c.main_func.autotvout_fm_27.dat"
#define AUTOTB_TVIN_fm_28 "../tv/cdatafile/c.main_func.autotvin_fm_28.dat"
#define AUTOTB_TVOUT_fm_28 "../tv/cdatafile/c.main_func.autotvout_fm_28.dat"
#define AUTOTB_TVIN_fm_29 "../tv/cdatafile/c.main_func.autotvin_fm_29.dat"
#define AUTOTB_TVOUT_fm_29 "../tv/cdatafile/c.main_func.autotvout_fm_29.dat"
#define AUTOTB_TVIN_fm_3 "../tv/cdatafile/c.main_func.autotvin_fm_3.dat"
#define AUTOTB_TVOUT_fm_3 "../tv/cdatafile/c.main_func.autotvout_fm_3.dat"
#define AUTOTB_TVIN_fm_30 "../tv/cdatafile/c.main_func.autotvin_fm_30.dat"
#define AUTOTB_TVOUT_fm_30 "../tv/cdatafile/c.main_func.autotvout_fm_30.dat"
#define AUTOTB_TVIN_fm_31 "../tv/cdatafile/c.main_func.autotvin_fm_31.dat"
#define AUTOTB_TVOUT_fm_31 "../tv/cdatafile/c.main_func.autotvout_fm_31.dat"
#define AUTOTB_TVIN_fm_4 "../tv/cdatafile/c.main_func.autotvin_fm_4.dat"
#define AUTOTB_TVOUT_fm_4 "../tv/cdatafile/c.main_func.autotvout_fm_4.dat"
#define AUTOTB_TVIN_fm_5 "../tv/cdatafile/c.main_func.autotvin_fm_5.dat"
#define AUTOTB_TVOUT_fm_5 "../tv/cdatafile/c.main_func.autotvout_fm_5.dat"
#define AUTOTB_TVIN_fm_6 "../tv/cdatafile/c.main_func.autotvin_fm_6.dat"
#define AUTOTB_TVOUT_fm_6 "../tv/cdatafile/c.main_func.autotvout_fm_6.dat"
#define AUTOTB_TVIN_fm_7 "../tv/cdatafile/c.main_func.autotvin_fm_7.dat"
#define AUTOTB_TVOUT_fm_7 "../tv/cdatafile/c.main_func.autotvout_fm_7.dat"
#define AUTOTB_TVIN_fm_8 "../tv/cdatafile/c.main_func.autotvin_fm_8.dat"
#define AUTOTB_TVOUT_fm_8 "../tv/cdatafile/c.main_func.autotvout_fm_8.dat"
#define AUTOTB_TVIN_fm_9 "../tv/cdatafile/c.main_func.autotvin_fm_9.dat"
#define AUTOTB_TVOUT_fm_9 "../tv/cdatafile/c.main_func.autotvout_fm_9.dat"
#define AUTOTB_TVIN_wt "../tv/cdatafile/c.main_func.autotvin_wt.dat"
#define AUTOTB_TVOUT_wt "../tv/cdatafile/c.main_func.autotvout_wt.dat"


// tvout file define:
#define AUTOTB_TVOUT_PC_fm "../tv/rtldatafile/rtl.main_func.autotvout_fm.dat"
#define AUTOTB_TVOUT_PC_fm_0 "../tv/rtldatafile/rtl.main_func.autotvout_fm_0.dat"
#define AUTOTB_TVOUT_PC_fm_1 "../tv/rtldatafile/rtl.main_func.autotvout_fm_1.dat"
#define AUTOTB_TVOUT_PC_fm_10 "../tv/rtldatafile/rtl.main_func.autotvout_fm_10.dat"
#define AUTOTB_TVOUT_PC_fm_11 "../tv/rtldatafile/rtl.main_func.autotvout_fm_11.dat"
#define AUTOTB_TVOUT_PC_fm_12 "../tv/rtldatafile/rtl.main_func.autotvout_fm_12.dat"
#define AUTOTB_TVOUT_PC_fm_13 "../tv/rtldatafile/rtl.main_func.autotvout_fm_13.dat"
#define AUTOTB_TVOUT_PC_fm_14 "../tv/rtldatafile/rtl.main_func.autotvout_fm_14.dat"
#define AUTOTB_TVOUT_PC_fm_15 "../tv/rtldatafile/rtl.main_func.autotvout_fm_15.dat"
#define AUTOTB_TVOUT_PC_fm_16 "../tv/rtldatafile/rtl.main_func.autotvout_fm_16.dat"
#define AUTOTB_TVOUT_PC_fm_17 "../tv/rtldatafile/rtl.main_func.autotvout_fm_17.dat"
#define AUTOTB_TVOUT_PC_fm_18 "../tv/rtldatafile/rtl.main_func.autotvout_fm_18.dat"
#define AUTOTB_TVOUT_PC_fm_19 "../tv/rtldatafile/rtl.main_func.autotvout_fm_19.dat"
#define AUTOTB_TVOUT_PC_fm_2 "../tv/rtldatafile/rtl.main_func.autotvout_fm_2.dat"
#define AUTOTB_TVOUT_PC_fm_20 "../tv/rtldatafile/rtl.main_func.autotvout_fm_20.dat"
#define AUTOTB_TVOUT_PC_fm_21 "../tv/rtldatafile/rtl.main_func.autotvout_fm_21.dat"
#define AUTOTB_TVOUT_PC_fm_22 "../tv/rtldatafile/rtl.main_func.autotvout_fm_22.dat"
#define AUTOTB_TVOUT_PC_fm_23 "../tv/rtldatafile/rtl.main_func.autotvout_fm_23.dat"
#define AUTOTB_TVOUT_PC_fm_24 "../tv/rtldatafile/rtl.main_func.autotvout_fm_24.dat"
#define AUTOTB_TVOUT_PC_fm_25 "../tv/rtldatafile/rtl.main_func.autotvout_fm_25.dat"
#define AUTOTB_TVOUT_PC_fm_26 "../tv/rtldatafile/rtl.main_func.autotvout_fm_26.dat"
#define AUTOTB_TVOUT_PC_fm_27 "../tv/rtldatafile/rtl.main_func.autotvout_fm_27.dat"
#define AUTOTB_TVOUT_PC_fm_28 "../tv/rtldatafile/rtl.main_func.autotvout_fm_28.dat"
#define AUTOTB_TVOUT_PC_fm_29 "../tv/rtldatafile/rtl.main_func.autotvout_fm_29.dat"
#define AUTOTB_TVOUT_PC_fm_3 "../tv/rtldatafile/rtl.main_func.autotvout_fm_3.dat"
#define AUTOTB_TVOUT_PC_fm_30 "../tv/rtldatafile/rtl.main_func.autotvout_fm_30.dat"
#define AUTOTB_TVOUT_PC_fm_31 "../tv/rtldatafile/rtl.main_func.autotvout_fm_31.dat"
#define AUTOTB_TVOUT_PC_fm_4 "../tv/rtldatafile/rtl.main_func.autotvout_fm_4.dat"
#define AUTOTB_TVOUT_PC_fm_5 "../tv/rtldatafile/rtl.main_func.autotvout_fm_5.dat"
#define AUTOTB_TVOUT_PC_fm_6 "../tv/rtldatafile/rtl.main_func.autotvout_fm_6.dat"
#define AUTOTB_TVOUT_PC_fm_7 "../tv/rtldatafile/rtl.main_func.autotvout_fm_7.dat"
#define AUTOTB_TVOUT_PC_fm_8 "../tv/rtldatafile/rtl.main_func.autotvout_fm_8.dat"
#define AUTOTB_TVOUT_PC_fm_9 "../tv/rtldatafile/rtl.main_func.autotvout_fm_9.dat"
#define AUTOTB_TVOUT_PC_wt "../tv/rtldatafile/rtl.main_func.autotvout_wt.dat"


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
  size_t divide_ceil(size_t a, size_t b)
  {
    return (a + b - 1) / b;
  }

  const bool little_endian()
  {
    int a = 1;
    return *(char*)&a == 1;
  }

  inline void rev_endian(unsigned char *p, size_t nbytes)
  {
    std::reverse(p, p+nbytes);
  }

  const bool LE = little_endian();

  inline size_t least_nbyte(size_t width)
  {
    return (width+7)>>3;
  }

  std::string formatData(unsigned char *pos, size_t wbits)
  {
    size_t wbytes = least_nbyte(wbits);
    size_t i = LE ? wbytes-1 : 0;
    auto next = [&] () {
      auto c = pos[i];
      LE ? --i : ++i;
      return c;
    };
    std::ostringstream ss;
    ss << "0x";
    if (int t = (wbits & 0x7)) {
      if (t <= 4) {
        unsigned char mask = (1<<t)-1;
        ss << std::hex << std::setfill('0') << std::setw(1)
           << (int) (next() & mask);
        wbytes -= 1;
      }
    }
    for (size_t i = 0; i < wbytes; ++i) {
      ss << std::hex << std::setfill('0') << std::setw(2) << (int)next();
    }
    return ss.str();
  }

  char ord(char c)
  {
    if (c >= 'a' && c <= 'f') {
      return c-'a'+10;
    } else if (c >= 'A' && c <= 'F') {
      return c-'A'+10;
    } else if (c >= '0' && c <= '9') {
      return c-'0';
    } else {
      throw SimException("Not Hexdecimal Digit", __LINE__);
    }
  }

  void unformatData(const char *data, unsigned char *put, size_t pbytes = 0)
  {
    size_t nchars = strlen(data+2);
    size_t nbytes = (nchars+1)>>1;
    if (pbytes == 0) {
      pbytes = nbytes;
    } else if (pbytes > nbytes) {
      throw SimException("Wrong size specified", __LINE__);
    }
    put = LE ? put : put+pbytes-1;
    auto nextp = [&] () {
      return LE ? put++ : put--;
    };
    const char *c = data + (nchars + 2) - 1;
    auto next = [&] () {
      char res { *c == 'x' ? (char)0 : ord(*c) };
      --c;
      return res;
    };
    for (size_t i = 0; i < pbytes; ++i) {
      char l = next();
      char h = next();
      *nextp() = (h<<4)+l;
    }
  }

  char* strip(char *s)
  {
    while (isspace(*s)) {
      ++s;
    }
    for (char *p = s+strlen(s)-1; p >= s; --p) {
      if (isspace(*p)) {
        *p = 0;
      } else {
        return s;
      }
    }
    return s;
  }

  size_t sum(const std::vector<size_t> &v)
  {
    size_t res = 0;
    for (const auto &e : v) {
      res += e;
    }
    return res;
  }

  const char* bad = "Bad TV file";
  const char* err = "Error on TV file";

  const unsigned char bmark[] = {
    0x5a, 0x5a, 0xa5, 0xa5, 0x0f, 0x0f, 0xf0, 0xf0
  };

#ifdef USE_BINARY_TV_FILE
  class Input {
    FILE *fp;
    long pos;

    void read(unsigned char *buf, size_t size)
    {
      if (fread(buf, size, 1, fp) != 1) {
        throw SimException(bad, __LINE__);
      }
      if (LE) {
        rev_endian(buf, size);
      }
    }

  public:
    void advance(size_t nbytes)
    {
      if (fseek(fp, nbytes, SEEK_CUR) == -1) {
        throw SimException(bad, __LINE__);
      }
    }

    Input(const char *path) : fp(nullptr)
    {
      fp = fopen(path, "rb");
      if (fp == nullptr) {
        errExit(__LINE__, err);
      }
    }

    void begin()
    {
      advance(8);
      pos = ftell(fp);
    }

    void reset()
    {
      fseek(fp, pos, SEEK_SET);
    }

    void into(unsigned char *param, size_t wbytes, size_t asize, size_t nbytes)
    {
      size_t n = nbytes / asize;
      size_t r = nbytes % asize;
      for (size_t i = 0; i < n; ++i) {
        read(param, wbytes);
        param += asize;
      }
      if (r > 0) {
        advance(asize-r);
        read(param, r);
      }
    }

    ~Input()
    {
      unsigned char buf[8];
      size_t res = fread(buf, 8, 1, fp);
      fclose(fp);
      if (res != 1) {
        errExit(__LINE__, bad);
      }
      if (std::memcmp(buf, bmark, 8) != 0) {
        errExit(__LINE__, bad);
      }
    }
  };

  class Output {
    FILE *fp;

    void write(unsigned char *buf, size_t size)
    {
      if (LE) {
        rev_endian(buf, size);
      }
      if (fwrite(buf, size, 1, fp) != 1) {
        throw SimException(err, __LINE__);
      }
      if (LE) {
        rev_endian(buf, size);
      }
    }

  public:
    Output(const char *path) : fp(nullptr)
    {
      fp = fopen(path, "wb");
      if (fp == nullptr) {
        errExit(__LINE__, err);
      }
    }

    void begin(size_t total)
    {
      unsigned char buf[8] = {0};
      std::memcpy(buf, &total, sizeof(buf));
      write(buf, sizeof(buf));
    }

    void from(unsigned char *param, size_t wbytes, size_t asize, size_t nbytes, size_t skip)
    {
      param -= asize*skip;
      size_t n = divide_ceil(nbytes, asize);
      for (size_t i = 0; i < n; ++i) {
        write(param, wbytes);
        param += asize;
      }
    }

    ~Output()
    {
      size_t res = fwrite(bmark, 8, 1, fp);
      fclose(fp);
      if (res != 1) {
        errExit(__LINE__, err);
      }
    }
  };
#endif

  class Reader {
    FILE *fp;
    long pos;
    int size;
    char *s;

    void readline()
    {
      s = fgets(s, size, fp);
      if (s == nullptr) {
        throw SimException(bad, __LINE__);
      }
    }

  public:
    Reader(const char *path) : fp(nullptr), size(1<<12), s(new char[size])
    {
      try {
        fp = fopen(path, "r");
        if (fp == nullptr) {
          throw SimException(err, __LINE__);
        } else {
          readline();
          static const char mark[] = "[[[runtime]]]\n";
          if (strcmp(s, mark) != 0) {
            throw SimException(bad, __LINE__);
          }
        }
      } catch (const hls::sim::SimException &e) {
        errExit(e.line, e.msg);
      }
    }

    ~Reader()
    {
      fclose(fp);
      delete[] s;
    }

    void begin()
    {
      readline();
      static const char mark[] = "[[transaction]]";
      if (strncmp(s, mark, strlen(mark)) != 0) {
        throw SimException(bad, __LINE__);
      }
      pos = ftell(fp);
    }

    void reset()
    {
      fseek(fp, pos, SEEK_SET);
    }

    void skip(size_t n)
    {
      for (size_t i = 0; i < n; ++i) {
        readline();
      }
    }

    char* next()
    {
      long pos = ftell(fp);
      readline();
      if (*s == '[') {
        fseek(fp, pos, SEEK_SET);
        return nullptr;
      }
      return strip(s);
    }

    void end()
    {
      do {
        readline();
      } while (strcmp(s, "[[/transaction]]\n") != 0);
    }
  };

  class Writer {
    FILE *fp;

    void write(const char *s)
    {
      if (fputs(s, fp) == EOF) {
        throw SimException(err, __LINE__);
      }
    }

  public:
    Writer(const char *path) : fp(nullptr)
    {
      try {
        fp = fopen(path, "w");
        if (fp == nullptr) {
          throw SimException(err, __LINE__);
        } else {
          static const char mark[] = "[[[runtime]]]\n";
          write(mark);
        }
      } catch (const hls::sim::SimException &e) {
        errExit(e.line, e.msg);
      }
    }

    virtual ~Writer()
    {
      try {
        static const char mark[] = "[[[/runtime]]]\n";
        write(mark);
      } catch (const hls::sim::SimException &e) {
        errExit(e.line, e.msg);
      }
      fclose(fp);
    }

    void begin(size_t AESL_transaction)
    {
      static const char mark[] = "[[transaction]]           ";
      write(mark);
      auto buf = std::to_string(AESL_transaction);
      buf.push_back('\n');
      buf.push_back('\0');
      write(buf.data());
    }

    void next(const char *s)
    {
      write(s);
      write("\n");
    }

    void end()
    {
      static const char mark[] = "[[/transaction]]\n";
      write(mark);
    }
  };

  bool RTLOutputCheckAndReplacement(char *data)
  {
    bool changed = false;
    for (size_t i = 2; i < strlen(data); ++i) {
      if (data[i] == 'X' || data[i] == 'x') {
        data[i] = '0';
        changed = true;
      }
    }
    return changed;
  }

  void warnOnX()
  {
    static const char msg[] =
      "WARNING: [SIM 212-201] RTL produces unknown value "
      "'x' or 'X' on some port, possible cause: "
      "There are uninitialized variables in the design.\n";
    fprintf(stderr, msg);
  }

#ifndef POST_CHECK
  class RefTCL {
    FILE *fp;
    std::ostringstream ss;

    void formatDepth()
    {
      ss << "set depth_list {\n";
      for (auto &p : depth) {
        ss << "  {" << p.first << " " << p.second << "}\n";
      }
      if (nameHBM != "") {
        ss << "  {" << nameHBM << " " << depthHBM << "}\n";
      }
      ss << "}\n";
    }

    void formatTransNum()
    {
      ss << "set trans_num " << AESL_transaction << "\n";
    }

    void formatHBM()
    {
      ss << "set HBM_ArgDict {\n"
         << "  Name " << nameHBM << "\n"
         << "  Port " << portHBM << "\n"
         << "  BitWidth " << widthHBM << "\n"
         << "}\n";
    }

    void close()
    {
      formatDepth();
      formatTransNum();
      if (nameHBM != "") {
        formatHBM();
      }
      std::string &&s { ss.str() };
      size_t res = fwrite(s.data(), s.size(), 1, fp);
      fclose(fp);
      if (res != 1) {
        errExit(__LINE__, err);
      }
    }

  public:
    std::map<const std::string, size_t> depth;
    std::string nameHBM;
    size_t depthHBM;
    std::string portHBM;
    unsigned widthHBM;
    size_t AESL_transaction;
    std::mutex mut;

    RefTCL(const char *path)
    {
      fp = fopen(path, "w");
      if (fp == nullptr) {
        errExit(__LINE__, err);
      }
    }

    void set(const char* name, size_t dep)
    {
      std::lock_guard<std::mutex> guard(mut);
      if (depth[name] < dep) {
        depth[name] = dep;
      }
    }

    ~RefTCL()
    {
      close();
    }
  };

#endif

  struct Register {
    const char* name;
    unsigned width;
#ifdef POST_CHECK
    Reader* reader;
#else
    Writer* owriter;
    Writer* iwriter;
#endif
    void* param;

#ifndef POST_CHECK
    void doTCL(RefTCL &tcl)
    {
      if (strcmp(name, "return") == 0) {
        tcl.set("ap_return", 1);
      } else {
        tcl.set(name, 1);
      }
    }
#endif
    ~Register()
    {
#ifdef POST_CHECK
      delete reader;
#else
      delete owriter;
      delete iwriter;
#endif
    }
  };

  template<typename Reader, typename Writer>
  struct Memory {
    unsigned width;
    unsigned asize;
    bool hbm;
    std::vector<const char*> name;
#ifdef POST_CHECK
    Reader* reader;
#else
    Writer* owriter;
    Writer* iwriter;
#endif
    std::vector<void*> param;
    std::vector<size_t> nbytes;
    std::vector<size_t> offset;
    std::vector<bool> hasWrite;

    size_t depth()
    {
      size_t depth = 0;
      for (size_t n : nbytes) {
        depth += divide_ceil(n, asize);
      }
      return depth;
    }

#ifndef POST_CHECK
    void doTCL(RefTCL &tcl)
    {
      if (hbm) {
        tcl.nameHBM.append(name[0]);
        tcl.portHBM.append("{").append(name[0]);
        for (size_t i = 1; i < name.size(); ++i) {
          tcl.nameHBM.append("_").append(name[i]);
          tcl.portHBM.append(" ").append(name[i]);
        }
        tcl.nameHBM.append("_HBM");
        tcl.portHBM.append("}");
        tcl.widthHBM = width;
        tcl.depthHBM = divide_ceil(nbytes[0], asize);
      } else {
        tcl.set(name[0], depth());
      }
    }
#endif

    ~Memory()
    {
#ifdef POST_CHECK
      delete reader;
#else
      delete owriter;
      delete iwriter;
#endif
    }
  };

  struct A2Stream {
    unsigned width;
    unsigned asize;
    const char* name;
#ifdef POST_CHECK
    Reader* reader;
#else
    Writer* owriter;
    Writer* iwriter;
#endif
    void* param;
    size_t nbytes;
    bool hasWrite;

#ifndef POST_CHECK
    void doTCL(RefTCL &tcl)
    {
      tcl.set(name, divide_ceil(nbytes, asize));
    }
#endif

    ~A2Stream()
    {
#ifdef POST_CHECK
      delete reader;
#else
      delete owriter;
      delete iwriter;
#endif
    }
  };

  template<typename E>
  struct Stream {
    unsigned width;
    const char* name;
#ifdef POST_CHECK
    Reader* reader;
#else
    Writer* writer;
    Writer* swriter;
    Writer* gwriter;
#endif
    hls::stream<E>* param;
    std::vector<E> buf;
    size_t initSize;
    size_t depth;
    bool hasWrite;

    void markSize()
    {
      initSize = param->size();
    }

    void buffer()
    {
      buf.clear();
      while (!param->empty()) {
        buf.push_back(param->read());
      }
      for (auto &e : buf) {
        param->write(e);
      }
    }

#ifndef POST_CHECK
    void doTCL(RefTCL &tcl)
    {
      tcl.set(name, depth);
    }
#endif

    ~Stream()
    {
#ifdef POST_CHECK
      delete reader;
#else
      delete writer;
      delete swriter;
      delete gwriter;
#endif
    }
  };

#ifdef POST_CHECK
  void check(Register &port)
  {
    port.reader->begin();
    bool foundX = false;
    if (char *s = port.reader->next()) {
      foundX |= RTLOutputCheckAndReplacement(s);
      unformatData(s, (unsigned char*)port.param);
    }
    port.reader->end();
    if (foundX) {
      warnOnX();
    }
  }

#ifdef USE_BINARY_TV_FILE
  void checkHBM(Memory<Input, Output> &port)
  {
    port.reader->begin();
    size_t wbytes = least_nbyte(port.width);
    for (size_t i = 0; i < port.param.size(); ++i) {
      if (port.hasWrite[i]) {
        port.reader->reset();
        size_t skip = wbytes * port.offset[i];
        port.reader->advance(skip);
        port.reader->into((unsigned char*)port.param[i], wbytes,
                           port.asize, port.nbytes[i] - skip);
      }
    }
  }

  void check(Memory<Input, Output> &port)
  {
    if (port.hbm) {
      return checkHBM(port);
    } else {
      port.reader->begin();
      size_t wbytes = least_nbyte(port.width);
      for (size_t i = 0; i < port.param.size(); ++i) {
        if (port.hasWrite[i]) {
          port.reader->into((unsigned char*)port.param[i], wbytes,
                             port.asize, port.nbytes[i]);
        } else {
          size_t n = divide_ceil(port.nbytes[i], port.asize);
          port.reader->advance(port.asize*n);
        }
      }
    }
  }
#endif
  void transfer(Reader *reader, size_t nbytes, unsigned char *put, bool &foundX)
  {
    if (char *s = reader->next()) {
      foundX |= RTLOutputCheckAndReplacement(s);
      unformatData(s, put, nbytes);
    } else {
      throw SimException("No more data", __LINE__);
    }
  }

  void checkHBM(Memory<Reader, Writer> &port)
  {
    port.reader->begin();
    bool foundX = false;
    size_t wbytes = least_nbyte(port.width);
    for (size_t i = 0, last = port.param.size()-1; i <= last; ++i) {
      if (port.hasWrite[i]) {
        port.reader->skip(port.offset[i]);
        size_t n = port.nbytes[i] / port.asize - port.offset[i];
        unsigned char *put = (unsigned char*)port.param[i];
        for (size_t j = 0; j < n; ++j) {
          transfer(port.reader, wbytes, put, foundX);
          put += port.asize;
        }
        if (i < last) {
          port.reader->reset();
        }
      }
    }
    port.reader->end();
    if (foundX) {
      warnOnX();
    }
  }

  void check(Memory<Reader, Writer> &port)
  {
    if (port.hbm) {
      return checkHBM(port);
    } else {
      port.reader->begin();
      bool foundX = false;
      size_t wbytes = least_nbyte(port.width);
      for (size_t i = 0; i < port.param.size(); ++i) {
        if (port.hasWrite[i]) {
          size_t n = port.nbytes[i] / port.asize;
          size_t r = port.nbytes[i] % port.asize;
          unsigned char *put = (unsigned char*)port.param[i];
          for (size_t j = 0; j < n; ++j) {
            transfer(port.reader, wbytes, put, foundX);
            put += port.asize;
          }
          if (r > 0) {
            transfer(port.reader, r, put, foundX);
          }
        } else {
          size_t n = divide_ceil(port.nbytes[i], port.asize);
          port.reader->skip(n);
        }
      }
      port.reader->end();
      if (foundX) {
        warnOnX();
      }
    }
  }

  void check(A2Stream &port)
  {
    port.reader->begin();
    bool foundX = false;
    if (port.hasWrite) {
      size_t wbytes = least_nbyte(port.width);
      size_t n = port.nbytes / port.asize;
      size_t r = port.nbytes % port.asize;
      unsigned char *put = (unsigned char*)port.param;
      for (size_t j = 0; j < n; ++j) {
        if (char *s = port.reader->next()) {
          foundX |= RTLOutputCheckAndReplacement(s);
          unformatData(s, put, wbytes);
        }
        put += port.asize;
      }
      if (r > 0) {
        if (char *s = port.reader->next()) {
          foundX |= RTLOutputCheckAndReplacement(s);
          unformatData(s, put, r);
        }
      }
    }
    port.reader->end();
    if (foundX) {
      warnOnX();
    }
  }

  template<typename E>
  void check(Stream<E> &port)
  {
    if (port.hasWrite) {
      port.reader->begin();
      bool foundX = false;
      E *p = new E;
      while (char *s = port.reader->next()) {
        foundX |= RTLOutputCheckAndReplacement(s);
        unformatData(s, (unsigned char*)p);
        port.param->write(*p);
      }
      delete p;
      port.reader->end();
      if (foundX) {
        warnOnX();
      }
    } else {
      port.reader->begin();
      size_t n = 0;
      if (char *s = port.reader->next()) {
        std::istringstream ss(s);
        ss >> n;
      } else {
        throw SimException(bad, __LINE__);
      }
      port.reader->end();
      for (size_t j = 0; j < n; ++j) {
        port.param->read();
      }
    }
  }
#else
  void dump(Register &port, Writer *writer, size_t AESL_transaction)
  {
    writer->begin(AESL_transaction);
    std::string &&s { formatData((unsigned char*)port.param, port.width) };
    writer->next(s.data());
    writer->end();
  }

  void error_on_depth_unspecified(const char *portName)
  {
    std::string msg {"A depth specification is required for MAXI interface port "};
    msg.append("'");
    msg.append(portName);
    msg.append("'");
    msg.append(" for cosimulation.");
    throw SimException(msg, __LINE__);
  }

#ifdef USE_BINARY_TV_FILE
  void dump(Memory<Input, Output> &port, Output *writer, size_t AESL_transaction)
  {
    writer->begin(port.depth());
    size_t wbytes = least_nbyte(port.width);
    for (size_t i = 0; i < port.param.size(); ++i) {
      if (port.nbytes[i] == 0) {
        error_on_depth_unspecified(port.hbm ? port.name[i] : port.name[0]);
      }
      writer->from((unsigned char*)port.param[i], wbytes, port.asize,
                   port.nbytes[i], 0);
    }
  }

#endif
  void dump(Memory<Reader, Writer> &port, Writer *writer, size_t AESL_transaction)
  {
    writer->begin(AESL_transaction);
    for (size_t i = 0; i < port.param.size(); ++i) {
      if (port.nbytes[i] == 0) {
        error_on_depth_unspecified(port.hbm ? port.name[i] : port.name[0]);
      }
      size_t n = divide_ceil(port.nbytes[i], port.asize);
      unsigned char *put = (unsigned char*)port.param[i];
      for (size_t j = 0; j < n; ++j) {
        std::string &&s {
          formatData(put, port.width)
        };
        writer->next(s.data());
        put += port.asize;
      }
      if (port.hbm) {
        break;
      }
    }
    writer->end();
  }

  void dump(A2Stream &port, Writer *writer, size_t AESL_transaction)
  {
    writer->begin(AESL_transaction);
    size_t n = divide_ceil(port.nbytes, port.asize);
    unsigned char *put = (unsigned char*)port.param;
    for (size_t j = 0; j < n; ++j) {
      std::string &&s { formatData(put, port.width) };
      writer->next(s.data());
      put += port.asize;
    }
    writer->end();
  }

  template<typename E>
  void dump(Stream<E> &port, size_t AESL_transaction)
  {
    if (port.hasWrite) {
      port.writer->begin(AESL_transaction);
      port.depth = port.param->size()-port.initSize;
      for (size_t j = 0; j < port.depth; ++j) {
        std::string &&s {
          formatData((unsigned char*)&port.buf[port.initSize+j], port.width)
        };
        port.writer->next(s.c_str());
      }
      port.writer->end();

      port.swriter->begin(AESL_transaction);
      port.swriter->next(std::to_string(port.depth).c_str());
      port.swriter->end();
    } else {
      port.writer->begin(AESL_transaction);
      port.depth = port.initSize-port.param->size();
      for (size_t j = 0; j < port.depth; ++j) {
        std::string &&s {
          formatData((unsigned char*)&port.buf[j], port.width)
        };
        port.writer->next(s.c_str());
      }
      port.writer->end();

      port.swriter->begin(AESL_transaction);
      port.swriter->next(std::to_string(port.depth).c_str());
      port.swriter->end();

      port.gwriter->begin(AESL_transaction);
      size_t n = (port.depth ? port.initSize : port.depth);
      size_t d = port.depth;
      do {
        port.gwriter->next(std::to_string(n--).c_str());
      } while (d--);
      port.gwriter->end();
    }
  }
#endif
}



extern "C"
void main_func_hw_stub_wrapper(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);

extern "C"
void apatb_main_func_hw(void* __xlx_apatb_param_input_feature_map, void* __xlx_apatb_param_layer_weights, void* __xlx_apatb_param_layer_bias, void* __xlx_apatb_param_output_feature_map_0, void* __xlx_apatb_param_output_feature_map_1, void* __xlx_apatb_param_output_feature_map_2, void* __xlx_apatb_param_output_feature_map_3, void* __xlx_apatb_param_output_feature_map_4, void* __xlx_apatb_param_output_feature_map_5, void* __xlx_apatb_param_output_feature_map_6, void* __xlx_apatb_param_output_feature_map_7, void* __xlx_apatb_param_output_feature_map_8, void* __xlx_apatb_param_output_feature_map_9, void* __xlx_apatb_param_output_feature_map_10, void* __xlx_apatb_param_output_feature_map_11, void* __xlx_apatb_param_output_feature_map_12, void* __xlx_apatb_param_output_feature_map_13, void* __xlx_apatb_param_output_feature_map_14, void* __xlx_apatb_param_output_feature_map_15, void* __xlx_apatb_param_output_feature_map_16, void* __xlx_apatb_param_output_feature_map_17, void* __xlx_apatb_param_output_feature_map_18, void* __xlx_apatb_param_output_feature_map_19, void* __xlx_apatb_param_output_feature_map_20, void* __xlx_apatb_param_output_feature_map_21, void* __xlx_apatb_param_output_feature_map_22, void* __xlx_apatb_param_output_feature_map_23, void* __xlx_apatb_param_output_feature_map_24, void* __xlx_apatb_param_output_feature_map_25, void* __xlx_apatb_param_output_feature_map_26, void* __xlx_apatb_param_output_feature_map_27, void* __xlx_apatb_param_output_feature_map_28, void* __xlx_apatb_param_output_feature_map_29, void* __xlx_apatb_param_output_feature_map_30, void* __xlx_apatb_param_output_feature_map_31)
{
  hls::sim::Byte<4> __xlx_offset_byte_param_input_feature_map;
  static hls::sim::Register port0 {
    .name = "input_feature_map",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_input_feature_map),
#endif
  };
  port0.param = &__xlx_offset_byte_param_input_feature_map;

  hls::sim::Byte<4> __xlx_offset_byte_param_layer_weights;
  static hls::sim::Register port1 {
    .name = "layer_weights",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_layer_weights),
#endif
  };
  port1.param = &__xlx_offset_byte_param_layer_weights;

  hls::sim::Byte<4> __xlx_offset_byte_param_layer_bias;
  static hls::sim::Register port2 {
    .name = "layer_bias",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_layer_bias),
#endif
  };
  port2.param = &__xlx_offset_byte_param_layer_bias;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_0;
  static hls::sim::Register port3 {
    .name = "output_feature_map_0",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_0),
#endif
  };
  port3.param = &__xlx_offset_byte_param_output_feature_map_0;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_1;
  static hls::sim::Register port4 {
    .name = "output_feature_map_1",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_1),
#endif
  };
  port4.param = &__xlx_offset_byte_param_output_feature_map_1;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_2;
  static hls::sim::Register port5 {
    .name = "output_feature_map_2",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_2),
#endif
  };
  port5.param = &__xlx_offset_byte_param_output_feature_map_2;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_3;
  static hls::sim::Register port6 {
    .name = "output_feature_map_3",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_3),
#endif
  };
  port6.param = &__xlx_offset_byte_param_output_feature_map_3;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_4;
  static hls::sim::Register port7 {
    .name = "output_feature_map_4",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_4),
#endif
  };
  port7.param = &__xlx_offset_byte_param_output_feature_map_4;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_5;
  static hls::sim::Register port8 {
    .name = "output_feature_map_5",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_5),
#endif
  };
  port8.param = &__xlx_offset_byte_param_output_feature_map_5;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_6;
  static hls::sim::Register port9 {
    .name = "output_feature_map_6",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_6),
#endif
  };
  port9.param = &__xlx_offset_byte_param_output_feature_map_6;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_7;
  static hls::sim::Register port10 {
    .name = "output_feature_map_7",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_7),
#endif
  };
  port10.param = &__xlx_offset_byte_param_output_feature_map_7;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_8;
  static hls::sim::Register port11 {
    .name = "output_feature_map_8",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_8),
#endif
  };
  port11.param = &__xlx_offset_byte_param_output_feature_map_8;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_9;
  static hls::sim::Register port12 {
    .name = "output_feature_map_9",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_9),
#endif
  };
  port12.param = &__xlx_offset_byte_param_output_feature_map_9;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_10;
  static hls::sim::Register port13 {
    .name = "output_feature_map_10",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_10),
#endif
  };
  port13.param = &__xlx_offset_byte_param_output_feature_map_10;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_11;
  static hls::sim::Register port14 {
    .name = "output_feature_map_11",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_11),
#endif
  };
  port14.param = &__xlx_offset_byte_param_output_feature_map_11;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_12;
  static hls::sim::Register port15 {
    .name = "output_feature_map_12",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_12),
#endif
  };
  port15.param = &__xlx_offset_byte_param_output_feature_map_12;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_13;
  static hls::sim::Register port16 {
    .name = "output_feature_map_13",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_13),
#endif
  };
  port16.param = &__xlx_offset_byte_param_output_feature_map_13;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_14;
  static hls::sim::Register port17 {
    .name = "output_feature_map_14",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_14),
#endif
  };
  port17.param = &__xlx_offset_byte_param_output_feature_map_14;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_15;
  static hls::sim::Register port18 {
    .name = "output_feature_map_15",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_15),
#endif
  };
  port18.param = &__xlx_offset_byte_param_output_feature_map_15;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_16;
  static hls::sim::Register port19 {
    .name = "output_feature_map_16",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_16),
#endif
  };
  port19.param = &__xlx_offset_byte_param_output_feature_map_16;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_17;
  static hls::sim::Register port20 {
    .name = "output_feature_map_17",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_17),
#endif
  };
  port20.param = &__xlx_offset_byte_param_output_feature_map_17;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_18;
  static hls::sim::Register port21 {
    .name = "output_feature_map_18",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_18),
#endif
  };
  port21.param = &__xlx_offset_byte_param_output_feature_map_18;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_19;
  static hls::sim::Register port22 {
    .name = "output_feature_map_19",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_19),
#endif
  };
  port22.param = &__xlx_offset_byte_param_output_feature_map_19;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_20;
  static hls::sim::Register port23 {
    .name = "output_feature_map_20",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_20),
#endif
  };
  port23.param = &__xlx_offset_byte_param_output_feature_map_20;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_21;
  static hls::sim::Register port24 {
    .name = "output_feature_map_21",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_21),
#endif
  };
  port24.param = &__xlx_offset_byte_param_output_feature_map_21;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_22;
  static hls::sim::Register port25 {
    .name = "output_feature_map_22",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_22),
#endif
  };
  port25.param = &__xlx_offset_byte_param_output_feature_map_22;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_23;
  static hls::sim::Register port26 {
    .name = "output_feature_map_23",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_23),
#endif
  };
  port26.param = &__xlx_offset_byte_param_output_feature_map_23;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_24;
  static hls::sim::Register port27 {
    .name = "output_feature_map_24",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_24),
#endif
  };
  port27.param = &__xlx_offset_byte_param_output_feature_map_24;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_25;
  static hls::sim::Register port28 {
    .name = "output_feature_map_25",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_25),
#endif
  };
  port28.param = &__xlx_offset_byte_param_output_feature_map_25;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_26;
  static hls::sim::Register port29 {
    .name = "output_feature_map_26",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_26),
#endif
  };
  port29.param = &__xlx_offset_byte_param_output_feature_map_26;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_27;
  static hls::sim::Register port30 {
    .name = "output_feature_map_27",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_27),
#endif
  };
  port30.param = &__xlx_offset_byte_param_output_feature_map_27;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_28;
  static hls::sim::Register port31 {
    .name = "output_feature_map_28",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_28),
#endif
  };
  port31.param = &__xlx_offset_byte_param_output_feature_map_28;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_29;
  static hls::sim::Register port32 {
    .name = "output_feature_map_29",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_29),
#endif
  };
  port32.param = &__xlx_offset_byte_param_output_feature_map_29;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_30;
  static hls::sim::Register port33 {
    .name = "output_feature_map_30",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_30),
#endif
  };
  port33.param = &__xlx_offset_byte_param_output_feature_map_30;

  hls::sim::Byte<4> __xlx_offset_byte_param_output_feature_map_31;
  static hls::sim::Register port34 {
    .name = "output_feature_map_31",
    .width = 32,
#ifdef POST_CHECK
#else
    .owriter = nullptr,
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_output_feature_map_31),
#endif
  };
  port34.param = &__xlx_offset_byte_param_output_feature_map_31;

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port35 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port35 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm" },
#ifdef POST_CHECK
#else
    .owriter = nullptr,
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm),
#endif
#endif
  };
  port35.param = { __xlx_apatb_param_input_feature_map };
  port35.nbytes = { 3072 };
  port35.offset = {  };
  port35.hasWrite = { false };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port36 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port36 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_0" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_0),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_0),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_0),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_0),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_0),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_0),
#endif
#endif
  };
  port36.param = { __xlx_apatb_param_output_feature_map_0 };
  port36.nbytes = { 96 };
  port36.offset = {  };
  port36.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port37 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port37 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_1" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_1),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_1),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_1),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_1),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_1),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_1),
#endif
#endif
  };
  port37.param = { __xlx_apatb_param_output_feature_map_1 };
  port37.nbytes = { 96 };
  port37.offset = {  };
  port37.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port38 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port38 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_10" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_10),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_10),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_10),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_10),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_10),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_10),
#endif
#endif
  };
  port38.param = { __xlx_apatb_param_output_feature_map_10 };
  port38.nbytes = { 96 };
  port38.offset = {  };
  port38.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port39 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port39 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_11" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_11),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_11),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_11),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_11),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_11),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_11),
#endif
#endif
  };
  port39.param = { __xlx_apatb_param_output_feature_map_11 };
  port39.nbytes = { 96 };
  port39.offset = {  };
  port39.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port40 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port40 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_12" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_12),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_12),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_12),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_12),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_12),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_12),
#endif
#endif
  };
  port40.param = { __xlx_apatb_param_output_feature_map_12 };
  port40.nbytes = { 96 };
  port40.offset = {  };
  port40.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port41 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port41 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_13" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_13),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_13),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_13),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_13),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_13),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_13),
#endif
#endif
  };
  port41.param = { __xlx_apatb_param_output_feature_map_13 };
  port41.nbytes = { 96 };
  port41.offset = {  };
  port41.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port42 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port42 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_14" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_14),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_14),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_14),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_14),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_14),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_14),
#endif
#endif
  };
  port42.param = { __xlx_apatb_param_output_feature_map_14 };
  port42.nbytes = { 96 };
  port42.offset = {  };
  port42.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port43 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port43 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_15" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_15),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_15),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_15),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_15),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_15),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_15),
#endif
#endif
  };
  port43.param = { __xlx_apatb_param_output_feature_map_15 };
  port43.nbytes = { 96 };
  port43.offset = {  };
  port43.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port44 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port44 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_16" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_16),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_16),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_16),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_16),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_16),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_16),
#endif
#endif
  };
  port44.param = { __xlx_apatb_param_output_feature_map_16 };
  port44.nbytes = { 96 };
  port44.offset = {  };
  port44.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port45 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port45 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_17" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_17),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_17),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_17),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_17),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_17),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_17),
#endif
#endif
  };
  port45.param = { __xlx_apatb_param_output_feature_map_17 };
  port45.nbytes = { 96 };
  port45.offset = {  };
  port45.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port46 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port46 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_18" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_18),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_18),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_18),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_18),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_18),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_18),
#endif
#endif
  };
  port46.param = { __xlx_apatb_param_output_feature_map_18 };
  port46.nbytes = { 96 };
  port46.offset = {  };
  port46.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port47 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port47 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_19" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_19),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_19),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_19),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_19),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_19),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_19),
#endif
#endif
  };
  port47.param = { __xlx_apatb_param_output_feature_map_19 };
  port47.nbytes = { 96 };
  port47.offset = {  };
  port47.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port48 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port48 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_2" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_2),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_2),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_2),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_2),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_2),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_2),
#endif
#endif
  };
  port48.param = { __xlx_apatb_param_output_feature_map_2 };
  port48.nbytes = { 96 };
  port48.offset = {  };
  port48.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port49 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port49 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_20" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_20),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_20),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_20),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_20),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_20),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_20),
#endif
#endif
  };
  port49.param = { __xlx_apatb_param_output_feature_map_20 };
  port49.nbytes = { 96 };
  port49.offset = {  };
  port49.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port50 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port50 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_21" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_21),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_21),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_21),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_21),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_21),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_21),
#endif
#endif
  };
  port50.param = { __xlx_apatb_param_output_feature_map_21 };
  port50.nbytes = { 96 };
  port50.offset = {  };
  port50.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port51 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port51 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_22" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_22),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_22),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_22),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_22),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_22),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_22),
#endif
#endif
  };
  port51.param = { __xlx_apatb_param_output_feature_map_22 };
  port51.nbytes = { 96 };
  port51.offset = {  };
  port51.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port52 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port52 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_23" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_23),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_23),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_23),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_23),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_23),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_23),
#endif
#endif
  };
  port52.param = { __xlx_apatb_param_output_feature_map_23 };
  port52.nbytes = { 96 };
  port52.offset = {  };
  port52.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port53 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port53 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_24" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_24),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_24),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_24),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_24),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_24),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_24),
#endif
#endif
  };
  port53.param = { __xlx_apatb_param_output_feature_map_24 };
  port53.nbytes = { 96 };
  port53.offset = {  };
  port53.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port54 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port54 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_25" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_25),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_25),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_25),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_25),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_25),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_25),
#endif
#endif
  };
  port54.param = { __xlx_apatb_param_output_feature_map_25 };
  port54.nbytes = { 96 };
  port54.offset = {  };
  port54.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port55 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port55 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_26" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_26),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_26),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_26),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_26),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_26),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_26),
#endif
#endif
  };
  port55.param = { __xlx_apatb_param_output_feature_map_26 };
  port55.nbytes = { 96 };
  port55.offset = {  };
  port55.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port56 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port56 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_27" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_27),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_27),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_27),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_27),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_27),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_27),
#endif
#endif
  };
  port56.param = { __xlx_apatb_param_output_feature_map_27 };
  port56.nbytes = { 96 };
  port56.offset = {  };
  port56.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port57 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port57 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_28" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_28),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_28),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_28),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_28),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_28),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_28),
#endif
#endif
  };
  port57.param = { __xlx_apatb_param_output_feature_map_28 };
  port57.nbytes = { 96 };
  port57.offset = {  };
  port57.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port58 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port58 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_29" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_29),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_29),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_29),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_29),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_29),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_29),
#endif
#endif
  };
  port58.param = { __xlx_apatb_param_output_feature_map_29 };
  port58.nbytes = { 96 };
  port58.offset = {  };
  port58.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port59 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port59 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_3" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_3),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_3),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_3),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_3),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_3),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_3),
#endif
#endif
  };
  port59.param = { __xlx_apatb_param_output_feature_map_3 };
  port59.nbytes = { 96 };
  port59.offset = {  };
  port59.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port60 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port60 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_30" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_30),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_30),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_30),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_30),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_30),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_30),
#endif
#endif
  };
  port60.param = { __xlx_apatb_param_output_feature_map_30 };
  port60.nbytes = { 96 };
  port60.offset = {  };
  port60.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port61 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port61 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_31" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_31),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_31),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_31),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_31),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_31),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_31),
#endif
#endif
  };
  port61.param = { __xlx_apatb_param_output_feature_map_31 };
  port61.nbytes = { 96 };
  port61.offset = {  };
  port61.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port62 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port62 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_4" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_4),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_4),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_4),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_4),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_4),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_4),
#endif
#endif
  };
  port62.param = { __xlx_apatb_param_output_feature_map_4 };
  port62.nbytes = { 96 };
  port62.offset = {  };
  port62.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port63 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port63 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_5" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_5),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_5),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_5),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_5),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_5),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_5),
#endif
#endif
  };
  port63.param = { __xlx_apatb_param_output_feature_map_5 };
  port63.nbytes = { 96 };
  port63.offset = {  };
  port63.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port64 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port64 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_6" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_6),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_6),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_6),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_6),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_6),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_6),
#endif
#endif
  };
  port64.param = { __xlx_apatb_param_output_feature_map_6 };
  port64.nbytes = { 96 };
  port64.offset = {  };
  port64.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port65 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port65 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_7" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_7),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_7),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_7),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_7),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_7),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_7),
#endif
#endif
  };
  port65.param = { __xlx_apatb_param_output_feature_map_7 };
  port65.nbytes = { 96 };
  port65.offset = {  };
  port65.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port66 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port66 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_8" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_8),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_8),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_8),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_8),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_8),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_8),
#endif
#endif
  };
  port66.param = { __xlx_apatb_param_output_feature_map_8 };
  port66.nbytes = { 96 };
  port66.offset = {  };
  port66.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port67 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port67 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "fm_9" },
#ifdef POST_CHECK
#ifdef USE_BINARY_TV_FILE
    .reader = new hls::sim::Input(AUTOTB_TVOUT_PC_fm_9),
#else
    .reader = new hls::sim::Reader(AUTOTB_TVOUT_PC_fm_9),
#endif
#else
#ifdef USE_BINARY_TV_FILE
    .owriter = new hls::sim::Output(AUTOTB_TVOUT_fm_9),
#else
    .owriter = new hls::sim::Writer(AUTOTB_TVOUT_fm_9),
#endif
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_fm_9),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_fm_9),
#endif
#endif
  };
  port67.param = { __xlx_apatb_param_output_feature_map_9 };
  port67.nbytes = { 96 };
  port67.offset = {  };
  port67.hasWrite = { true };

#ifdef USE_BINARY_TV_FILE
  static hls::sim::Memory<hls::sim::Input, hls::sim::Output> port68 {
#else
  static hls::sim::Memory<hls::sim::Reader, hls::sim::Writer> port68 {
#endif
    .width = 16,
    .asize = 2,
    .hbm = false,
    .name = { "wt" },
#ifdef POST_CHECK
#else
    .owriter = nullptr,
#ifdef USE_BINARY_TV_FILE
    .iwriter = new hls::sim::Output(AUTOTB_TVIN_wt),
#else
    .iwriter = new hls::sim::Writer(AUTOTB_TVIN_wt),
#endif
#endif
  };
  __xlx_offset_byte_param_layer_weights = 0*2;
  __xlx_offset_byte_param_layer_bias = 81*2;
  port68.param = { __xlx_apatb_param_layer_weights, __xlx_apatb_param_layer_bias };
  port68.nbytes = { 162, 6 };
  port68.offset = { 0, 81 };
  port68.hasWrite = { false, false };

  try {
#ifdef POST_CHECK
    CodeState = ENTER_WRAPC_PC;
    check(port36);
    check(port37);
    check(port38);
    check(port39);
    check(port40);
    check(port41);
    check(port42);
    check(port43);
    check(port44);
    check(port45);
    check(port46);
    check(port47);
    check(port48);
    check(port49);
    check(port50);
    check(port51);
    check(port52);
    check(port53);
    check(port54);
    check(port55);
    check(port56);
    check(port57);
    check(port58);
    check(port59);
    check(port60);
    check(port61);
    check(port62);
    check(port63);
    check(port64);
    check(port65);
    check(port66);
    check(port67);
#else
    static hls::sim::RefTCL tcl("../tv/cdatafile/ref.tcl");
    CodeState = DUMP_INPUTS;
    dump(port0, port0.iwriter, tcl.AESL_transaction);
    dump(port1, port1.iwriter, tcl.AESL_transaction);
    dump(port2, port2.iwriter, tcl.AESL_transaction);
    dump(port3, port3.iwriter, tcl.AESL_transaction);
    dump(port4, port4.iwriter, tcl.AESL_transaction);
    dump(port5, port5.iwriter, tcl.AESL_transaction);
    dump(port6, port6.iwriter, tcl.AESL_transaction);
    dump(port7, port7.iwriter, tcl.AESL_transaction);
    dump(port8, port8.iwriter, tcl.AESL_transaction);
    dump(port9, port9.iwriter, tcl.AESL_transaction);
    dump(port10, port10.iwriter, tcl.AESL_transaction);
    dump(port11, port11.iwriter, tcl.AESL_transaction);
    dump(port12, port12.iwriter, tcl.AESL_transaction);
    dump(port13, port13.iwriter, tcl.AESL_transaction);
    dump(port14, port14.iwriter, tcl.AESL_transaction);
    dump(port15, port15.iwriter, tcl.AESL_transaction);
    dump(port16, port16.iwriter, tcl.AESL_transaction);
    dump(port17, port17.iwriter, tcl.AESL_transaction);
    dump(port18, port18.iwriter, tcl.AESL_transaction);
    dump(port19, port19.iwriter, tcl.AESL_transaction);
    dump(port20, port20.iwriter, tcl.AESL_transaction);
    dump(port21, port21.iwriter, tcl.AESL_transaction);
    dump(port22, port22.iwriter, tcl.AESL_transaction);
    dump(port23, port23.iwriter, tcl.AESL_transaction);
    dump(port24, port24.iwriter, tcl.AESL_transaction);
    dump(port25, port25.iwriter, tcl.AESL_transaction);
    dump(port26, port26.iwriter, tcl.AESL_transaction);
    dump(port27, port27.iwriter, tcl.AESL_transaction);
    dump(port28, port28.iwriter, tcl.AESL_transaction);
    dump(port29, port29.iwriter, tcl.AESL_transaction);
    dump(port30, port30.iwriter, tcl.AESL_transaction);
    dump(port31, port31.iwriter, tcl.AESL_transaction);
    dump(port32, port32.iwriter, tcl.AESL_transaction);
    dump(port33, port33.iwriter, tcl.AESL_transaction);
    dump(port34, port34.iwriter, tcl.AESL_transaction);
    dump(port35, port35.iwriter, tcl.AESL_transaction);
    dump(port36, port36.iwriter, tcl.AESL_transaction);
    dump(port37, port37.iwriter, tcl.AESL_transaction);
    dump(port38, port38.iwriter, tcl.AESL_transaction);
    dump(port39, port39.iwriter, tcl.AESL_transaction);
    dump(port40, port40.iwriter, tcl.AESL_transaction);
    dump(port41, port41.iwriter, tcl.AESL_transaction);
    dump(port42, port42.iwriter, tcl.AESL_transaction);
    dump(port43, port43.iwriter, tcl.AESL_transaction);
    dump(port44, port44.iwriter, tcl.AESL_transaction);
    dump(port45, port45.iwriter, tcl.AESL_transaction);
    dump(port46, port46.iwriter, tcl.AESL_transaction);
    dump(port47, port47.iwriter, tcl.AESL_transaction);
    dump(port48, port48.iwriter, tcl.AESL_transaction);
    dump(port49, port49.iwriter, tcl.AESL_transaction);
    dump(port50, port50.iwriter, tcl.AESL_transaction);
    dump(port51, port51.iwriter, tcl.AESL_transaction);
    dump(port52, port52.iwriter, tcl.AESL_transaction);
    dump(port53, port53.iwriter, tcl.AESL_transaction);
    dump(port54, port54.iwriter, tcl.AESL_transaction);
    dump(port55, port55.iwriter, tcl.AESL_transaction);
    dump(port56, port56.iwriter, tcl.AESL_transaction);
    dump(port57, port57.iwriter, tcl.AESL_transaction);
    dump(port58, port58.iwriter, tcl.AESL_transaction);
    dump(port59, port59.iwriter, tcl.AESL_transaction);
    dump(port60, port60.iwriter, tcl.AESL_transaction);
    dump(port61, port61.iwriter, tcl.AESL_transaction);
    dump(port62, port62.iwriter, tcl.AESL_transaction);
    dump(port63, port63.iwriter, tcl.AESL_transaction);
    dump(port64, port64.iwriter, tcl.AESL_transaction);
    dump(port65, port65.iwriter, tcl.AESL_transaction);
    dump(port66, port66.iwriter, tcl.AESL_transaction);
    dump(port67, port67.iwriter, tcl.AESL_transaction);
    dump(port68, port68.iwriter, tcl.AESL_transaction);
    port0.doTCL(tcl);
    port1.doTCL(tcl);
    port2.doTCL(tcl);
    port3.doTCL(tcl);
    port4.doTCL(tcl);
    port5.doTCL(tcl);
    port6.doTCL(tcl);
    port7.doTCL(tcl);
    port8.doTCL(tcl);
    port9.doTCL(tcl);
    port10.doTCL(tcl);
    port11.doTCL(tcl);
    port12.doTCL(tcl);
    port13.doTCL(tcl);
    port14.doTCL(tcl);
    port15.doTCL(tcl);
    port16.doTCL(tcl);
    port17.doTCL(tcl);
    port18.doTCL(tcl);
    port19.doTCL(tcl);
    port20.doTCL(tcl);
    port21.doTCL(tcl);
    port22.doTCL(tcl);
    port23.doTCL(tcl);
    port24.doTCL(tcl);
    port25.doTCL(tcl);
    port26.doTCL(tcl);
    port27.doTCL(tcl);
    port28.doTCL(tcl);
    port29.doTCL(tcl);
    port30.doTCL(tcl);
    port31.doTCL(tcl);
    port32.doTCL(tcl);
    port33.doTCL(tcl);
    port34.doTCL(tcl);
    port35.doTCL(tcl);
    port36.doTCL(tcl);
    port37.doTCL(tcl);
    port38.doTCL(tcl);
    port39.doTCL(tcl);
    port40.doTCL(tcl);
    port41.doTCL(tcl);
    port42.doTCL(tcl);
    port43.doTCL(tcl);
    port44.doTCL(tcl);
    port45.doTCL(tcl);
    port46.doTCL(tcl);
    port47.doTCL(tcl);
    port48.doTCL(tcl);
    port49.doTCL(tcl);
    port50.doTCL(tcl);
    port51.doTCL(tcl);
    port52.doTCL(tcl);
    port53.doTCL(tcl);
    port54.doTCL(tcl);
    port55.doTCL(tcl);
    port56.doTCL(tcl);
    port57.doTCL(tcl);
    port58.doTCL(tcl);
    port59.doTCL(tcl);
    port60.doTCL(tcl);
    port61.doTCL(tcl);
    port62.doTCL(tcl);
    port63.doTCL(tcl);
    port64.doTCL(tcl);
    port65.doTCL(tcl);
    port66.doTCL(tcl);
    port67.doTCL(tcl);
    port68.doTCL(tcl);
    CodeState = CALL_C_DUT;
    main_func_hw_stub_wrapper(__xlx_apatb_param_input_feature_map, __xlx_apatb_param_layer_weights, __xlx_apatb_param_layer_bias, __xlx_apatb_param_output_feature_map_0, __xlx_apatb_param_output_feature_map_1, __xlx_apatb_param_output_feature_map_2, __xlx_apatb_param_output_feature_map_3, __xlx_apatb_param_output_feature_map_4, __xlx_apatb_param_output_feature_map_5, __xlx_apatb_param_output_feature_map_6, __xlx_apatb_param_output_feature_map_7, __xlx_apatb_param_output_feature_map_8, __xlx_apatb_param_output_feature_map_9, __xlx_apatb_param_output_feature_map_10, __xlx_apatb_param_output_feature_map_11, __xlx_apatb_param_output_feature_map_12, __xlx_apatb_param_output_feature_map_13, __xlx_apatb_param_output_feature_map_14, __xlx_apatb_param_output_feature_map_15, __xlx_apatb_param_output_feature_map_16, __xlx_apatb_param_output_feature_map_17, __xlx_apatb_param_output_feature_map_18, __xlx_apatb_param_output_feature_map_19, __xlx_apatb_param_output_feature_map_20, __xlx_apatb_param_output_feature_map_21, __xlx_apatb_param_output_feature_map_22, __xlx_apatb_param_output_feature_map_23, __xlx_apatb_param_output_feature_map_24, __xlx_apatb_param_output_feature_map_25, __xlx_apatb_param_output_feature_map_26, __xlx_apatb_param_output_feature_map_27, __xlx_apatb_param_output_feature_map_28, __xlx_apatb_param_output_feature_map_29, __xlx_apatb_param_output_feature_map_30, __xlx_apatb_param_output_feature_map_31);
    CodeState = DUMP_OUTPUTS;
    dump(port36, port36.owriter, tcl.AESL_transaction);
    dump(port37, port37.owriter, tcl.AESL_transaction);
    dump(port38, port38.owriter, tcl.AESL_transaction);
    dump(port39, port39.owriter, tcl.AESL_transaction);
    dump(port40, port40.owriter, tcl.AESL_transaction);
    dump(port41, port41.owriter, tcl.AESL_transaction);
    dump(port42, port42.owriter, tcl.AESL_transaction);
    dump(port43, port43.owriter, tcl.AESL_transaction);
    dump(port44, port44.owriter, tcl.AESL_transaction);
    dump(port45, port45.owriter, tcl.AESL_transaction);
    dump(port46, port46.owriter, tcl.AESL_transaction);
    dump(port47, port47.owriter, tcl.AESL_transaction);
    dump(port48, port48.owriter, tcl.AESL_transaction);
    dump(port49, port49.owriter, tcl.AESL_transaction);
    dump(port50, port50.owriter, tcl.AESL_transaction);
    dump(port51, port51.owriter, tcl.AESL_transaction);
    dump(port52, port52.owriter, tcl.AESL_transaction);
    dump(port53, port53.owriter, tcl.AESL_transaction);
    dump(port54, port54.owriter, tcl.AESL_transaction);
    dump(port55, port55.owriter, tcl.AESL_transaction);
    dump(port56, port56.owriter, tcl.AESL_transaction);
    dump(port57, port57.owriter, tcl.AESL_transaction);
    dump(port58, port58.owriter, tcl.AESL_transaction);
    dump(port59, port59.owriter, tcl.AESL_transaction);
    dump(port60, port60.owriter, tcl.AESL_transaction);
    dump(port61, port61.owriter, tcl.AESL_transaction);
    dump(port62, port62.owriter, tcl.AESL_transaction);
    dump(port63, port63.owriter, tcl.AESL_transaction);
    dump(port64, port64.owriter, tcl.AESL_transaction);
    dump(port65, port65.owriter, tcl.AESL_transaction);
    dump(port66, port66.owriter, tcl.AESL_transaction);
    dump(port67, port67.owriter, tcl.AESL_transaction);
    tcl.AESL_transaction++;
#endif
  } catch (const hls::sim::SimException &e) {
    hls::sim::errExit(e.line, e.msg);
  }
}