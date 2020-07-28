#include <iostream>
#include <unistd.h>
#include <time.h>
#include <string>
#include <chrono>
#include <ctime>
#include <map>
#include <sstream>
#include <vector>
#include <fstream>


// Edits and comments by Graham Greig
// This function obtains the information in the register readback and saves it into a
// semi-parsed string.
// Note: All ROOT functions are removed out at this time.
int st_decode_abc_star_chip_register_packet(int module_id, unsigned int *regAddress, unsigned int *regValue, std::string* data, std::ofstream *outputfile)
{
  int error = 0;
  // Checks if HCC is present and throws and error if it is. This should not return zero but
  // Should return a string that gives the suspected error report.
  if (e->HccPresent())
  {
    std::cout << "st_decode_abc_star_chip_register: This function doesn't support HCCStar\n";
    //return 1;
    // Add string save feature.
    *outputfile << "Did not expect an HCC. This could be and SEE." << std::endl;
  }
  // "link" 1 should be the same
  int l = 0;

  // Get the width of the scan.
  int scan_width = e->m[module_id]->scan_size(e->burst_count, l);
  int offset = 0;
  
  while (!e->m[module_id]->scan_lookup(e->burst_count, l, offset))
  {
    offset++;
    if (offset >= scan_width)
      *outputfile << "Only recieved zeros for this register. Length is " << offset << " for register " << *regAddress << std::endl;
    error = 1;
  }

  if (e->debug_level > 1)
  {
    printf("Using offset %d for %d\n", offset, module_id);
  }

  // This should not return and the bits should be captured. However, a string could be returned.
  // This should probably be changed to 68 bits. Unsure why it was set to 70...
  if (offset + 70 >= scan_width)
  {
    *outputfile << "Recieved less bits than expected." << std::endl;
  }

  // Changed this to a while loop that iterates until the entire scan is accumulated.
  // The spacing could still be useful as it tells where bits are expected.
  // Debug loop prints values to the console.
  if (e->debug_level > 5)
  {
    for (int b = 0; b < 70; b++)
    {
      /*if ((b + offset) >= scan_width)
      {
        break;
      }*/
      int bit = e->m[module_id]->scan_lookup(e->burst_count, l, offset + b);
      printf("%d", bit);
      // 111 0100 00100000 0000 {11110000 00000000 00000111 11111111} {00100001 01000001} 0
      if (((b == 2) || (b == 6) || b == 14 || b == 18 || b == 50 || b == 66))
        printf(" ");
    }
    printf("\n");
  }

  // Here is where the data is saved out to a string.
  // May need to add a case for data longer than expected!
  unsigned int accumulate = 0;
  for (int b = 0; b < 70; b++)
  {
    // keeping this for now but may remove.
    if ((b + offset) >= scan_width)
    {
      break;
    }
    int bit = e->m[module_id]->scan_lookup(e->burst_count, l, offset + b);

    // Get the data string and format.
    data->append(std::to_string(bit));
    if (((b == 2) || (b == 6) || b == 14 || b == 18 || b == 50 || b == 66))
      data->append(" ");
    
    // Unsure if this will still work... needs testing.
    accumulate = (accumulate << 1) + (bit ? 1 : 0);

    //        printf("reg acc: b = %d, %d, %d\n", b, bit, accumulate);
    switch (b)
    {
    case 2:
      // Start bits
      if (accumulate != 7)
      {
        *outputfile << "Bad start bits " << accumulate << std::endl;
        error = 1;
      }
      accumulate = 0;
      break;
    case 6:
      if (accumulate != 4)
      {
        *outputfile << "Unexpected type for register read is " << accumulate << std::endl;
        error = 1;
      }
      accumulate = 0;
      break;
    case 18:
      if (accumulate != 0)
        *outputfile << "Non-zero status" << std::endl;
      error = 1;
      accumulate = 0;
      break;
    case 14:
      *regAddress = accumulate;
      accumulate = 0;
      break;
    case 50:
      *regValue = accumulate;
      accumulate = 0;
      break;
    case 66: /*printf("Status is %x\n", accumulate);*/
      accumulate = 0;
      break;
    default:
      break;
    }
  }
  return error;
}

void RegisterReadBack_V0(const std::string &runname, int nloops, double interval = 1, bool debug = false)
{
  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  // SETUP
  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  // Trying to dump the raw data for comparison... unsure if this will work without testing.
  e->burst.dump_raw_data = true;

  //Defining register map.
  std::string hw_chip_name = "ABC";
  std::map<int, std::string> hw_regs = {{1, "DCS1"}, {2, "DCS2"}, {3, "DCS3"}, {4, "DCS4"}, {6, "DCS6"}, 
                                    {7, "DCS7"}, {32, "Config0"}, {33, "Config1"}, {34, "Config2"}, 
                                    {35, "Config3"}, {36, "Config4"}, {37, "Config5"}, {38, "Config6"}, 
                                    {48, "SEUStat"}, {49, "SEUStat2"}, {50, "FuseStat"}, {51, "ADCStat"},
                                    {52, "LCBErr"}, {63, "HPR"}};

  for (int i = 0; i < 8; i++)
  {
    std::stringstream ss;
    ss << "Mask" << i;
    hw_regs.insert({0x10 + i, ss.str()});
  }
  for (int i = 0; i < 32; i++)
  {
    std::stringstream ss;
    ss << "TrimLo" << i;
    hw_regs.insert({0x40 + i, ss.str()});
  }
  for (int i = 0; i < 8; i++)
  {
    std::stringstream ss;
    ss << "TrimHi" << i;
    hw_regs.insert({0x60 + i, ss.str()});
  }
  for (int i = 0; i < 8; i++)
  {
    std::stringstream ss;
    ss << "CalMask" << i;
    hw_regs.insert({0x68 + i, ss.str()});
  }
  for (int i = 0; i < 64; i++)
  {
    std::stringstream ss;
    ss << "Counter" << i;
    hw_regs.insert({0x80 + i, ss.str()}); 
  }

  //Setup the timer
  time_t timer;
  int starttime = time(&timer);
  struct tm *timeinfo;
  std::chrono::time_point<std::chrono::system_clock> start, currentTime; 
  start = std::chrono::system_clock::now(); 
  
// Should this still be used... Need to investigate.
  abc_star_hpr_stop(); // to make the hit pattern read out from the L0buffer consistent
  
  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  // MAIN TEST LOOP
  // - Here, individual output files are created for the runlog and data. 
  // - Runs are 10 test iterations long.
  // - "nloops" determines the length of the overall test time.
  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  
  std::ofstream outputfile2(("runlog_" + runname + ".txt").c_str());
  for (int l = 0; l < nloops; l++)
  {
    abc_star_seu_reset();
    std::ofstream outputfile((runname + ".txt").c_str());

    const uint32_t sizeofmap = hw_regs.size();

    int totalhit_link0, totalhit_link1, event, run, loop, nfail;
    int size = (int)hw_regs.size();
    bool l0bufferfill;
    std::vector<std::string> rawdata[sizeofmap];
    std::string names[16] =
        {
            //ABCStar
            "GND_SEN",
            "NTC1_Probe",
            "NTC0",
            "TESTRES",
            "TESTCOM",
            "GNDD_Local",
            "AMUXOUT",
            "GNDA_Local",
            "VDDD_RAW_SENSE",
            "VDDD_REG_SENSE",
            "VDDA_RAW_SENSE",
            "VDDA_REG_SENSE",
            "IDDD",
            "IDDA",
            "VDDD",
            "VDDA",
        };
    double fmc1701_values[16] = {0}; //These values are only being saved to root... need to mod!!!
    
    // Cross section determiniation.
    l0bufferfill = true;
    if (l % 2 == 0)
      l0bufferfill = false;
    // Headder variables
    run = e->runnum;
    loop = l;

    // Write of header for output file. 
    outputfile << "RUN NUMBER: " << run << std::endl;
    outputfile << "LOOP NUMBER: " << loop << std::endl;
    outputfile << "SETTINGS" << std::endl;
    outputfile << "L0 Buffer Fill: " << l0bufferfill << std::endl;
    outputfile << "L0 Mask: " << !l0bufferfill << std::endl;
    outputfile << "TrimDAC: " << !l0bufferfill << std::endl;
    outputfile << "CalMask: " << !l0bufferfill << std::endl;
    outputfile << "" << std::endl;

    //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // INTERNAL TEST LOOP
    // - Commands are sent to configure individual registers.
    // - Resisters are readout and saved to the active data file.
    //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    const int nreadings = 10;
    for (int k = 0; k < nreadings; k++)
    {
      e->HsioFlush();
      if (debug)
        std::cout << "File " << l << " register reading iteration " << k << std::endl;
      event = l * nreadings + k;
      //timeinfo = localtime(&timer);
      //timestamp = timeinfo->tm_sec + timeinfo->tm_min * 100 + timeinfo->tm_hour * 10000;
      //t1 = time(&timer);
      currentTime = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsedTime = currentTime - start; 

      outputfile << "Event Number: " << event << std::endl;
      outputfile << "Elapsed Time: " << elapsedTime.count() << std::endl;

      //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      // REGISTER WRITE
      // 'l0bufferfill' 
      // TRUE: Send 0's to applicable registers to look for a 0->1 cross section.
      // This will unmask the L0 buffer and record hits.
      //
      // FALSE: Send 1's to the applicable registers for a 1->0 cross section.
      // This will mask the L0 buffer.
      // 
      // Note: Raw event data will have the opposite cross section.
      //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

      // Mask or unmask the L0 buffer and send a burst.
      abc_star_fill_l0(l0bufferfill);
      e->burst_ntrigs = 255;
      abc_star_counter_burst();

      // Set the TrimDAC registers based on 'l0bufferfill'
      if (l0bufferfill)
      {
        e->ConfigureVariable(ST_TRIM, 0);
        e->ConfigureVariable(ST_MASK, 0); // all 0s
      }
      else
      {
        e->ConfigureVariable(ST_TRIM, 31);
        e->ConfigureVariable(ST_MASK, 3); // all 1s
      }

      // Set the CalMask registers
      int regVal = l0bufferfill?0xffff:0;
      for (int n = 0; n < 8; n++)
      {
        e->ConfigureVariable(ST_ABC_STAR_RAW_REG_LO_BASE + 104 + n, regVal);
        e->ConfigureVariable(ST_ABC_STAR_RAW_REG_HI_BASE + 104 + n, regVal);
      }
      e->ExecuteConfigs();

      // Read out from the L0 buffer. This will be dumped to the raw data file.
      // *** Need to open this data file and extract the raw data to ensure it is not overwritten. ***
      abc_star_scanL0Buffer(127, 4, 100);
      totalhit_link0 = e->m[0]->scan_sum[0];
      totalhit_link1 = e->m[0]->scan_sum[1];
      abc_star_scanL0Buffer(127, 1, 9);

      
      //timeinfo = localtime(&timer);
      //timestamp2 = timeinfo->tm_sec + timeinfo->tm_min * 100 + timeinfo->tm_hour * 10000;
      //seconds0 = time(&timer) - starttime; /* get current time; same as: timer = time(NULL)  */ 
      elapsedTime = std::chrono::system_clock::now() - start - elapsedTime;
      outputfile << "Write Time: " << elapsedTime.count() << std::endl;

      if (debug)
      {
        std::cout << ctime(&timer) << std::endl;
      }

      nfail = 0;
      outputfile << "REGISTER DATA" << std::endl;

      //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      // REGISTER READ
      // Iterate over the map of registers. Read the registers and check for failures.
      //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      int reg_index = 0;
      std::map<int, std::string>::iterator it_reg;
      for (it_reg = hw_regs.begin(); it_reg != hw_regs.end(); it_reg++)
      {
        int regId = it_reg->first;
        std::string regName = it_reg->second;
        if (debug)
          std::cout << " Reading Register ID " << regId << std::endl;

        e->ReadChipRegister(regId);
        unsigned int regAddress = 0;
        unsigned int regValue = 0;
        // Single ABC, doesn't have an index
        unsigned int chipIndex = 0;
        int error = 1;
        int n = 0;           // index for module ;
        long accumulate = 0; // packet
        std::string data = ""; //String to store data.

        if (debug)
        {
          std::cout << std::endl;
          std::cout << "Reading Register " << regName << std::endl;
        }

        // Data parser.
        error = st_decode_abc_star_chip_register_packet(n, &regAddress, &regValue, &data, &outputfile);
        
        // Commenting this out for now as I belive it terminates loops early...
        /*if (did_fail != 0)
        {
          e->HsioFlush();
          nfail++;
        }*/
        
        //Output register values to data file. 
        outputfile << "Register Address: " << regId << " Register Name: " << regName << std::endl;
        outputfile << "Recieved Register Address: " << regAddress <<  std::endl;
        outputfile << "Recieved Data Packet (LONG): " << regValue <<  std::endl;
        outputfile << "Binary Data: " <<  data << std::endl;
        elapsedTime = std::chrono::system_clock::now() - start - elapsedTime;
        outputfile << "Read Time: " << elapsedTime.count() << std::endl;
        outputfile << "" << std::endl;
        reg_index++;
      }

      //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      // FMC1701 Read
      //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      std::vector<double> data_ina230ReadIV = f->ina230ReadIV(0);
      std::vector<double> muxdata = f->max11617ReadOneChip(0);

      outputfile << "FMC1701 Values: " << std::endl;
      for (int a = 0; a < (int)data_ina230ReadIV.size(); a++)
      {
        //fmc1701_values[a + 12] = data_ina230ReadIV[a];
        outputfile << names[a + 12] << ": " << data_ina230ReadIV[a] << std::endl;
      }

      for (int a = 0; a < (int)muxdata.size(); a++)
      {
        fmc1701_values[a] = muxdata[a];
        outputfile << names[a] << ": " << muxdata[a] << std::endl;
      }

      //seconds = time(&timer) - starttime; /* get current time; same as: timer = time(NULL)  */
      //t1 = time(&timer) - t1;

      outputfile2 << std::endl;
      outputfile2 << setw(20) << "Event Meta Data - Run " << setw(5) << run << setw(15) << " fileNumber " << l << setw(15) << " eventNumber " << event << setw(25) << setw(25) << std::endl;
      outputfile2 << "FMC I/V readings - VDDA_RAW_SENSE " << fmc1701_values[8] << " V, VDDA_RAW_SENSE " << fmc1701_values[9] << " V, IDDD " << fmc1701_values[12] << " mA, IDDA " << fmc1701_values[13] << " mA" << std::endl;
      outputfile2 << "L0buffer total hit numbers - even bank " << totalhit_link0 << " odd bank " << totalhit_link1 << " expected hit = 65536 = 256*256 when filled, 0 even not filled. L0buffer fill status" << l0bufferfill << std::endl;
      outputfile2 << "Total number of register read errors " << nfail << std::endl;

      abc_star_reg_reset();
      star_chip_fast_command(7, 0);
    }

    std::cout << "File closed here " << std::endl;

    //std::string command = ".! mv " + runname + ".root " + jobid;
    //std::cout << " Outputing file " << runname << "_" << l << ".root" << std::endl;

    /*std::string jobid_txt = (runname + "_" + std::to_string(l) + ".txt");
    std::string command_txt = ".! mv " + runname + ".txt " + jobid_txt;

    gROOT->ProcessLine(command.c_str());
    gROOT->ProcessLine(command_txt.c_str());*/
    std::cout << " Outputing file " << runname << "_" << l << ".txt" << std::endl;
    
  }
}
