#include <unistd.h>
#include <time.h>
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>

/*#include <TTree.h>
#include <TFile.h>
#include <TROOT.h> */

// Pretty sure this is never used...
/*void packet_parser(long x)
{
  vector<int> ret;
  while (x)
  {
    if (x & 1)
      ret.push_back(1);
    else
      ret.push_back(0);
    x >>= 1;
  }
  std::reverse(ret.begin(), ret.end());
  for (int i = 0; i < ret.size(); i++)
  {
    cout << ret[i];
    if (((i == 2) || (i == 6) || i == 14 || i == 18 || i == 50 || i == 66))
      printf(" ");
  }
  cout << endl;
}*/

/* Haichen Wang - save the packet as an long */
//int st_decode_abc_star_chip_register_packet(int module_id, unsigned char *regAddress, unsigned int *regValue, vector<int> *packet, ofstream *outputfile)

// Edits and comments by Graham Greig
// This function obtains the information in the register readback and saves it into a
// semi-parsed string.
// Note: All ROOT functions are commented out at this time.
int st_decode_abc_star_chip_register_packet(int module_id, unsigned char *regAddress, unsigned int *regValue, std::string *data, ofstream *outputfile)
{
  int error = 0;
  // Checks if HCC is present and throws and error if it is. This should not return zero but
  // Should return a string that gives the suspected error report.
  if (e->HccPresent())
  {
    std::cout << "st_decode_abc_star_chip_register: This function doesn't support HCCStar\n";
    //return 1;
    // Add string save feature.
    *outputfile << "Did not expect an HCC. This could be and SEE." << endl;
  }
  // "link" 1 should be the same
  int l = 0;

  // Get the width of the scan.
  int scan_width = e->m[module_id]->scan_size(e->burst_count, l);

  int offset = 0;
  // This determines the .
  while (!e->m[module_id]->scan_lookup(e->burst_count, l, offset))
  {
    offset++;
    if (offset >= scan_width)
      *outputfile << "Only recieved zeros for this register. Length is " << offset << " for register " << *regAddress << endl;
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
    *outputfile << "Recieved less bits than expected." << endl;
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
    *data.append(to_string(bit));
    if (((b == 2) || (b == 6) || b == 14 || b == 18 || b == 50 || b == 66))
      *data.append(" ");
    
    // Unsure if this will still work... needs testing.
    accumulate = (accumulate << 1) + (bit ? 1 : 0);

    //        printf("reg acc: b = %d, %d, %d\n", b, bit, accumulate);
    switch (b)
    {
    case 2:
      // Start bits
      if (accumulate != 7)
      {
        *outputfile << "Bad start bits " << accumulate << endl;
        error = 1;
      }
      accumulate = 0;
      break;
    case 6:
      if (accumulate != 4)
      {
        *outputfile << "Unexpected type for register read is " << accumulate << endl;
        error = 1;
      }
      accumulate = 0;
      break;
    case 18:
      if (accumulate != 0)
        *outputfile << "Non-zero status" << endl;
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
  //  *accumulated = accumulate_2;
  return error;
}

void RegisterReadBack(const std::string &runname, int nloops, double interval = 1, bool debug = false)
{
  // Trying to dump the raw data for comparison... unsure if this will work without testing.
  burst.dump_raw_data = true;

  //Defining register map.
  std::map<int, std::string> hw_regs;
  std::string hw_chip_name = "";

  std::map<int, std::string> regs = {{1, "DCS1"}, {2, "DCS2"}, {3, "DCS3"}, {4, "DCS4"}, {6, "DCS6"}, {7, "DCS7"}, {32, "Config0"}, {33, "Config1"}, {34, "Config2"}, {35, "Config3"}, {36, "Config4"}, {37, "Config5"}, {38, "Config6"}, {48, "SEUStat"}, {49, "SEUStat2"}, {50, "FuseStat"}, {51, "ADCStat"}, {52, "LCBErr"}, {63, "HPR"}};

  regs.insert({16, "Mask0"});
  regs.insert({17, "Mask1"});
  regs.insert({18, "Mask2"});
  regs.insert({19, "Mask3"});
  regs.insert({20, "Mask4"});
  regs.insert({21, "Mask5"});
  regs.insert({22, "Mask6"});
  regs.insert({23, "Mask7"});

  for (int i = 0; i < 32; i++)
  {
    std::stringstream ss;
    ss << "TrimLo" << i;
    regs.insert({0x40 + i, ss.str()});
  }
  for (int i = 0; i < 8; i++)
  {
    std::stringstream ss;
    ss << "TrimHi" << i;
    regs.insert({0x60 + i, ss.str()});
  }
  for (int i = 0; i < 8; i++)
  {
    std::stringstream ss;
    ss << "CalMask" << i;
    regs.insert({0x68 + i, ss.str()});
  }
  for (int i = 0; i < 64; i++)
  {
    std::stringstream ss;
    ss << "Counter" << i;
    regs.insert({0x80 + i, ss.str()}); // per Bruce's email
  }

  hw_regs = regs;
  hw_chip_name = "ABC";

  time_t timer;

  int starttime = time(&timer);
  struct tm *timeinfo;

  abc_star_hpr_stop(); // to make the hit pattern read out from the L0buffer consistent
  
  ofstream outputfile2(("runlog_" + runname + ".txt").c_str());
  for (int l = 0; l < nloops; l++)
  {
    abc_star_seu_reset();

    

    ofstream outputfile((runname + ".txt").c_str());

    /*TFile ff((runname + ".root").c_str(), "recreate");
    TTree tree("tree", "tree");
    uint32_t seconds, seconds0, timestamp, timestamp2, t1;
    int totalhit_link0, totalhit_link1, event, run, loop, nfail;
    bool l0bufferfill;
    tree.Branch("seconds", &seconds, "seconds/I");
    tree.Branch("event", &event, "event/I");
    tree.Branch("run", &run, "run/I");
    tree.Branch("loop", &loop, "loop/I");
    tree.Branch("t1", &t1, "t1/I");
    tree.Branch("nfail", &nfail, "nfail/I");
    tree.Branch("seconds0", &seconds0, "seconds0/I");
    tree.Branch("timestamp", &timestamp, "timestamp/I");
    tree.Branch("timestamp2", &timestamp2, "timestamp2/I");

    tree.Branch("l0bufferfill", &l0bufferfill, "l0bufferfill/B");

    tree.Branch("totalhit_link0", &totalhit_link0, "totalhit_link0/I");
    tree.Branch("totalhit_link1", &totalhit_link1, "totalhit_link1/I");*/

    // EDIT: Made change to tree structure S.T. data is saved as a string.
    const uint32_t sizeofmap = hw_regs.size();
    uint32_t value_of_register[sizeofmap];
    uint32_t value_of_fail[sizeofmap];
    //      long packet_of_register    [sizeofmap];
    //vector<int> packet[sizeofmap];
    vector<std::string> rawdata[sizeofmap];

   /* int index = 0;
    std::map<int, std::string>::iterator it;
    for (it = hw_regs.begin(); it != hw_regs.end(); it++)
    {
      std::string regName = it->second;
      std::string suffix = "/i";

      
      tree.Branch(regName.c_str(), &value_of_register[index], (regName + suffix).c_str());
      tree.Branch((regName + "_fail").c_str(), &value_of_fail[index], (regName + "_fail" + suffix).c_str());
      //tree.Branch((regName + "_packet").c_str(), &packet[index]);
      tree.Branch((regName + "_rawdata").c_str(), &rawdata[index]);
      index++;
    }*/

    //fmc1701_max11617_names + ina230_name+ fmc1701_ad7997_names
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
    /*for (int k = 0; k < 16; k++)
    {
      tree.Branch(names[k].c_str(), &fmc1701_values[k], (names[k] + "/D").c_str());
    }*/


    int size = (int)hw_regs.size();

    const int nreadings = 10;
    for (int k = 0; k < nreadings; k++)
    {
      e->HsioFlush();
      if (debug)
        cout << "File " << l << " register reading iteration " << k << endl;
      run = e->runnum;
      loop = l;
      event = l * nreadings + k;
      timeinfo = localtime(&timer);
      timestamp = timeinfo->tm_sec + timeinfo->tm_min * 100 + timeinfo->tm_hour * 10000;
      t1 = time(&timer);
      unsigned int microseconds = interval * 1e6;

      l0bufferfill = true;
      if (l % 2 == 0)
        l0bufferfill = false;

      //	  abc_star_fill_l0(true);
      // This perfroms ST_SEND_MASK. Sends 0s if true and 1s if false.
      // 1s mask all channels 0s allow all hits.
      // Odd loops send 0s (no mask), even loops send 1s (all masked).
      // My guess is this allows for a test of 1->0 and 0->0 to see if this
      // has a different cross section.
      abc_star_fill_l0(l0bufferfill);

      // Sets the mask register again. This will allow hits on all channels.
      e->ConfigureVariable(ST_SEND_MASK, 1);
      e->ConfigureVariable(ST_NMASK, 0);
      e->ExecuteConfigs();

      // Sends a burst to the channels. burst_ntrigs determines the number of times the burst is sent.
      e->burst_ntrigs = 255;
      abc_star_counter_burst();

      // Reset the mask register to all 1s. This will mask out any further hits.
      // This also performs a 0->1 flip which would not have been tested.
      // Should we be sending a read command in between each test???
      e->ConfigureVariable(ST_SEND_MASK, 1);
      e->ConfigureVariable(ST_NMASK, 256);
      e->ExecuteConfigs();

      // If no mask was sent earlier, Send ST_TRIM(31) and ST_MASK(3).
      // Believe this is setting the 5-bit Trim_DAC register full or empty.
      // Unsure what ST_MASK does but belive it is setting all on and all off here.
      if (l0bufferfill)
      {
        e->ConfigureVariable(ST_TRIM, 31);
        e->ConfigureVariable(ST_MASK, 3); // all on
      }
      else
      {
        e->ConfigureVariable(ST_TRIM, 0);
        e->ConfigureVariable(ST_MASK, 0); // all off
      }

      // This is used to set "CalMask" registers?
      // Unsure which ones are being set so will need to investigate further.
      for (int n = 0; n < 8; n++)
      {

        e->ConfigureVariable(ST_ABC_STAR_RAW_REG_LO_BASE + 104 + n, 0);
        e->ConfigureVariable(ST_ABC_STAR_RAW_REG_HI_BASE + 104 + n, 0xffff);
      }
      e->ExecuteConfigs();

      //	  abc_star_counter_burst_test();
      // Read out from the L0 buffer. Experiment with the inputs to see how this affects the readout.
      abc_star_scanL0Buffer(127, 4, 100);
      totalhit_link0 = e->m[0]->scan_sum[0];
      totalhit_link1 = e->m[0]->scan_sum[1];
      abc_star_scanL0Buffer(127, 1, 9);

      //	  usleep(microseconds);

      timeinfo = localtime(&timer);
      timestamp2 = timeinfo->tm_sec + timeinfo->tm_min * 100 + timeinfo->tm_hour * 10000;

      seconds0 = time(&timer) - starttime; /* get current time; same as: timer = time(NULL)  */

      if (debug)
      {
        cout << ctime(&timer) << endl;
      }

      outputfile << run << " " << event << " " << timestamp << endl;

      nfail = 0;

      // Iterate over the map of registers. Read the registers and check for failures.
      // This needs to be modified so that the raw data is being dumped out.
      // Setting dump_raw_data to true should save the data to a file in the data directory.
      int reg_index = 0;
      std::map<int, std::string>::iterator it_reg;
      for (it_reg = hw_regs.begin(); it_reg != hw_regs.end(); it_reg++)
      {
        int regId = it_reg->first;
        std::string regName = it_reg->second;
        if (debug)
          cout << " Reading Register ID " << regId << endl;

        e->ReadChipRegister(regId);
        unsigned char regAddress = 0;
        unsigned int regValue = 0;
        // Single ABC, doesn't have an index
        unsigned int chipIndex = 0;
        int error = 1;
        int n = 0;           // index for module ;
        long accumulate = 0; // packet
        std::string data = ""; //String to store data.

        if (debug)
        {
          cout << endl;
          cout << "Reading Register " << regName << endl;
        }

        //vector<int> packet_of_register;
        // This is where data is being lost! Need to just get the raw data into an output file!!!!!!!!!!
        error = st_decode_abc_star_chip_register_packet(n, &regAddress, &regValue, &data, &outputfile);
        value_of_fail[reg_index] = error;
        value_of_register[reg_index] = regValue;
        rawdata[reg_index] = data;
        if (did_fail != 0)
        {
          e->HsioFlush();
          nfail++;
        }
        
        //packet[reg_index] = packet_of_register;

        /*for (int k = 0; k < packet_of_register.size(); k++)
        {
          outputfile << packet_of_register[k];
          if (((k == 2) || (k == 6) || k == 14 || k == 18 || k == 50 || k == 66))
            outputfile << " ";
        }*/
        outputfile << data << endl;
        reg_index++;
      }

      std::vector<double> data_ina230ReadIV = f->ina230ReadIV(0);
      std::vector<double> muxdata = f->max11617ReadOneChip(0);

      // NEED TO FIND A PLACE TO WRITE THESE!!!
      for (int a = 0; a < (int)data_ina230ReadIV.size(); a++)
      {
        fmc1701_values[a + 12] = data_ina230ReadIV[a];
      }

      for (int a = 0; a < (int)muxdata.size(); a++)
      {
        fmc1701_values[a] = muxdata[a];
      }

      seconds = time(&timer) - starttime; /* get current time; same as: timer = time(NULL)  */
      t1 = time(&timer) - t1;

      outputfile2 << std::endl;
      outputfile2 << setw(20) << "Event Meta Data - Run " << setw(5) << run << setw(15) << " fileNumber " << l << setw(15) << " eventNumber " << event << setw(25) << " timestamp " << timestamp << setw(25) << " single measurement time " << t1 << std::endl;
      outputfile2 << "FMC I/V readings - VDDA_RAW_SENSE " << fmc1701_values[8] << " V, VDDA_RAW_SENSE " << fmc1701_values[9] << " V, IDDD " << fmc1701_values[12] << " mA, IDDA " << fmc1701_values[13] << " mA" << std::endl;

      outputfile2 << "L0buffer total hit numbers - even bank " << totalhit_link0 << " odd bank " << totalhit_link1 << " expected hit = 65536 = 256*256 when filled, 0 even not filled. L0buffer fill status" << l0bufferfill << std::endl;
      if (l0bufferfill && (totalhit_link0 != 65536 || totalhit_link1 != 65536))
        outputfile2 << "L0buffer filled, but readout hit pattern doesn't match the expectation " << std::endl;
      outputfile2 << "Total number of register read errors " << nfail << std::endl;
     // tree.Fill();

      abc_star_reg_reset();
      star_chip_fast_command(7, 0);
    }

    /* ff.cd();
    cout << ff.GetName() << endl;
    tree.Write();
    ff.Close();*/

    cout << "File closed here " << endl;

    std::string command = ".! mv " + runname + ".root " + jobid;
    gROOT->ProcessLine(command.c_str());
    gROOT->ProcessLine(command_txt.c_str());
    std::cout << " Outputing file " << runname << "_" << l << ".root" << std::endl;

    std::string jobid_txt = (runname + "_" + std::to_string(l) + ".txt");
    std::string command_txt = ".! mv " + runname + ".txt " + jobid_txt;
    std::cout << " Outputing file " << runname << "_" << l << ".txt" << std::endl;
  }
}
