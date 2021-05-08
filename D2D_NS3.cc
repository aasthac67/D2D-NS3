/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * NIST-developed software is provided by NIST as a public
 * service. You may use, copy and distribute copies of the software in
 * any medium, provided that you keep intact this entire notice. You
 * may improve, modify and create derivative works of the software or
 * any portion of the software, and you may copy and distribute such
 * modifications or works. Modified works should carry a notice
 * stating that you changed the software and should note the date and
 * nature of any such change. Please explicitly acknowledge the
 * National Institute of Standards and Technology as the source of the
 * software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES
 * NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY
 * OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
 * WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED
 * OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT
 * WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 * SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 * CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of
 * using and distributing the software and you assume all risks
 * associated with its use, including but not limited to the risks and
 * costs of program errors, compliance with applicable laws, damage to
 * or loss of data, programs or equipment, and the unavailability or
 * interruption of operation. This software is not intended to be used
 * in any situation where a failure could cause risk of injury or
 * damage to property. The software developed by NIST employees is not
 * subject to copyright protection within the United States.
 */


#include "ns3/lte-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include <cfloat>
#include <sstream>

using namespace ns3;

void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon);

// This trace will log packet transmissions and receptions from the application
// layer.  The parameter 'localAddrs' is passed to this trace in case the
// address passed by the trace is not set (i.e., is '0.0.0.0' or '::').  The
// trace writes to a file stream provided by the first argument; by default,
// this trace file is 'ACN_Project.tr'
void
UePacketTrace (Ptr<OutputStreamWrapper> stream, const Address &localAddrs, std::string context, Ptr<const Packet> p, const Address &srcAddrs, const Address &dstAddrs)
{
  std::ostringstream oss;
  *stream->GetStream () << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << context << "\t" << p->GetSize () << "\t";
  if (InetSocketAddress::IsMatchingType (srcAddrs))
    {
      oss << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ();
      if (!oss.str ().compare ("0.0.0.0")) //srcAddrs not set
        {
          *stream->GetStream () << Ipv4Address::ConvertFrom (localAddrs) << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
        }
      else
        {
          oss.str ("");
          oss << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 ();
          if (!oss.str ().compare ("0.0.0.0")) //dstAddrs not set
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" <<  Ipv4Address::ConvertFrom (localAddrs) << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
        }
    }
  else if (Inet6SocketAddress::IsMatchingType (srcAddrs))
    {
      oss << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 ();
      if (!oss.str ().compare ("::")) //srcAddrs not set
        {
          *stream->GetStream () << Ipv6Address::ConvertFrom (localAddrs) << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
        }
      else
        {
          oss.str ("");
          oss << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 ();
          if (!oss.str ().compare ("::")) //dstAddrs not set
            {
              *stream->GetStream () << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Ipv6Address::ConvertFrom (localAddrs) << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
        }
    }
  else
    {
      *stream->GetStream () << "Unknown address type!" << std::endl;
    }
}
//till here its only for packet tracing

//Class for PSC UE
class d2d_PSC {
        public :
        
        NodeContainer d2dPSC;
        int Node_id ;
        int application_type;
        Vector position;
        
        void store(Vector pos, int id) {
                Node_id = id;
                application_type = 1;
                position.x = pos.x;
                position.y = pos.y;
                position.z = pos.z;
        }
};

//Class for Commercial UE
class d2d_Comm {
        public :
        
        NodeContainer d2dComm;
        int Node_id ;
        int application_type;
        Vector position;
        
        void store(Vector pos, int id) {
                Node_id = id;
                application_type = 2;
                position.x = pos.x;
                position.y = pos.y;
                position.z = pos.z;
        }
};

//Function for finding the average value of SINR
double avg_sinr=0;
double no_sinrs=0;

void PrintSinrTrace(std::string context ,uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
{ 
      //std::cout<<context<<std::endl;
      //std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
      //std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<10*log(sinr)/log(10)<<std::endl;
      //std::cout<<"\n";
      no_sinrs++;
      avg_sinr += 10*log(sinr)/log(10);
}

Time Jitter;
Time Delay;

NS_LOG_COMPONENT_DEFINE ("LteSlInCovrgCommMode1");

int main (int argc, char *argv[])
{
  Time simTime = Seconds (5);
  bool enableNsLogs = false;
  bool useIPv6 = false;
  float eNbPower = 30.0;

  std::string animFile = "ACN_proj_animation.xml" ;

  CommandLine cmd;
  cmd.AddValue ("eNbPower","eNB TxPower", eNbPower);
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("enableNsLogs", "Enable ns-3 logging (debug builds)", enableNsLogs);
  cmd.AddValue ("useIPv6", "Use IPv6 instead of IPv4", useIPv6);
  cmd.Parse (argc, argv);

  // Configure the scheduler
  Config::SetDefault ("ns3::RrSlFfMacScheduler::Itrp", UintegerValue (0));
  //The number of RBs allocated per UE for Sidelink
  Config::SetDefault ("ns3::RrSlFfMacScheduler::SlGrantSize", UintegerValue (5)); //5 RBs per sub frame will be used for sidelink transmission

  //Set the frequency

  //setting DL frequency for eNb as well as UE
  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (100));
  Config::SetDefault ("ns3::LteUeNetDevice::DlEarfcn", UintegerValue (100));
  //for UL
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", UintegerValue (18100));
  //DL and UL bandwidth set as 50 mbps
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (50));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (50));
  //Set error models
  Config::SetDefault ("ns3::LteSpectrumPhy::SlCtrlErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::SlDataErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::DropRbOnCollisionEnabled", BooleanValue (false));

  //parsing the command line arguments
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  //parse again so we can override input file default values via command line
  cmd.Parse (argc, argv);

  if (enableNsLogs)
    {
      LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);

      LogComponentEnable ("LteUeRrc", logLevel);
      LogComponentEnable ("LteUeMac", logLevel);
      LogComponentEnable ("LteSpectrumPhy", logLevel);
      LogComponentEnable ("LteUePhy", logLevel);
      LogComponentEnable ("LteEnbPhy", logLevel);
    }

  //Set the UEs power in dBm
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (23.0)); //Tried to change this value
  //Set the eNBs power in dBm
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (eNbPower)); //Tried to change this value

  //Sidelink bearers activation time
  Time slBearersActivationTime = Seconds (2.0);

  //Create the helpers
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  //Create and set the EPC helper
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  //Create Sidelink helper and set lteHelper
  Ptr<LteSidelinkHelper> proseHelper = CreateObject<LteSidelinkHelper> ();
  proseHelper->SetLteHelper (lteHelper);

  //Set pathloss model
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));

  //Enable Sidelink
  lteHelper->SetAttribute ("UseSidelink", BooleanValue (true)); //at eNbs side we are allowing Side link communication

  //Sidelink Round Robin scheduler
  lteHelper->SetSchedulerType ("ns3::RrSlFfMacScheduler");

  //Create nodes (eNb + UEs)
  NodeContainer enbNode;
  enbNode.Create (1);
  NS_LOG_UNCOND ("eNb node id = " << enbNode.Get (0)->GetId ());

  //Create 5 pairs of PSC and Commercial application nodes as well as set the application type as 1 for PSC and 2 for Commercial applications
  d2d_PSC psc[5]; 
  d2d_Comm comm[5];

  for(int i=0 ;i<5 ;i++) {
    psc[i].d2dPSC.Create (2);
    psc[i].application_type = 1;
    NS_LOG_UNCOND("PSC Node 0 ID = "<< psc[i].d2dPSC.Get(0)->GetId() );
    NS_LOG_UNCOND("PSC Node 1 ID = "<< psc[i].d2dPSC.Get(1)->GetId() );
  }
  for(int i=0 ;i<5 ;i++) {
    comm[i].d2dComm.Create (2);
    comm[i].application_type = 2;
    NS_LOG_UNCOND("Comm Node 0 ID = "<< comm[i].d2dComm.Get(0)->GetId() );
    NS_LOG_UNCOND("Comm Node 1 ID = "<< comm[i].d2dComm.Get(1)->GetId() );
  }

  //Position of the nodes
  Ptr<ListPositionAllocator> positionAllocEnb = CreateObject<ListPositionAllocator> ();
  positionAllocEnb->Add (Vector (40.0, 20.0, 0.0));

  //PSC
  Ptr<ListPositionAllocator> positionAllocPSC1 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC1->Add (Vector (10.0, 10.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocPSC2 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC2->Add (Vector (15.0, 10.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocPSC3 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC3->Add (Vector (20.0, 10.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocPSC4 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC4->Add (Vector (25.0, 15.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocPSC5 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC5->Add (Vector (22.0, 20.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocPSC6 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC6->Add (Vector (20.0, 25.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocPSC7 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC7->Add (Vector (15.0, 30.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocPSC8 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC8->Add (Vector (10.0, 25.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocPSC9 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC9->Add (Vector (7.0, 20.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocPSC10 = CreateObject<ListPositionAllocator> ();
  positionAllocPSC10->Add (Vector (5.0, 15.0, 0.0));

  //Commercial
  Ptr<ListPositionAllocator> positionAllocComm1 = CreateObject<ListPositionAllocator> ();
  positionAllocComm1->Add (Vector (55.0, 10.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocComm2 = CreateObject<ListPositionAllocator> ();
  positionAllocComm2->Add (Vector (60.0, 10.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocComm3 = CreateObject<ListPositionAllocator> ();
  positionAllocComm3->Add (Vector (65.0, 10.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocComm4 = CreateObject<ListPositionAllocator> ();
  positionAllocComm4->Add (Vector (70.0, 15.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocComm5 = CreateObject<ListPositionAllocator> ();
  positionAllocComm5->Add (Vector (67.0, 20.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocComm6 = CreateObject<ListPositionAllocator> ();
  positionAllocComm6->Add (Vector (65.0, 25.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocComm7 = CreateObject<ListPositionAllocator> ();
  positionAllocComm7->Add (Vector (60.0, 30.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocComm8 = CreateObject<ListPositionAllocator> ();
  positionAllocComm8->Add (Vector (55.0, 25.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocComm9 = CreateObject<ListPositionAllocator> ();
  positionAllocComm9->Add (Vector (52.0, 20.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocComm10 = CreateObject<ListPositionAllocator> ();
  positionAllocComm10->Add (Vector (50.0, 15.0, 0.0));

  //Install mobility
  MobilityHelper mobilityeNodeB;
  mobilityeNodeB.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityeNodeB.SetPositionAllocator (positionAllocEnb);
  mobilityeNodeB.Install (enbNode);

  //PSC
  MobilityHelper mobilityPSC1;
  mobilityPSC1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC1.SetPositionAllocator (positionAllocPSC1);
  mobilityPSC1.Install (psc[0].d2dPSC.Get (0));

  MobilityHelper mobilityPSC2;
  mobilityPSC2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC2.SetPositionAllocator (positionAllocPSC2);
  mobilityPSC2.Install (psc[0].d2dPSC.Get (1));

  MobilityHelper mobilityPSC3;
  mobilityPSC3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC3.SetPositionAllocator (positionAllocPSC3);
  mobilityPSC3.Install (psc[1].d2dPSC.Get (0));

  MobilityHelper mobilityPSC4;
  mobilityPSC4.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC4.SetPositionAllocator (positionAllocPSC4);
  mobilityPSC4.Install (psc[1].d2dPSC.Get (1));

  MobilityHelper mobilityPSC5;
  mobilityPSC5.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC5.SetPositionAllocator (positionAllocPSC5);
  mobilityPSC5.Install (psc[2].d2dPSC.Get (0));

  MobilityHelper mobilityPSC6;
  mobilityPSC6.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC6.SetPositionAllocator (positionAllocPSC6);
  mobilityPSC6.Install (psc[2].d2dPSC.Get (1));

  MobilityHelper mobilityPSC7;
  mobilityPSC7.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC7.SetPositionAllocator (positionAllocPSC7);
  mobilityPSC7.Install (psc[3].d2dPSC.Get (0));

  MobilityHelper mobilityPSC8;
  mobilityPSC8.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC8.SetPositionAllocator (positionAllocPSC8);
  mobilityPSC8.Install (psc[3].d2dPSC.Get (1));

  MobilityHelper mobilityPSC9;
  mobilityPSC9.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC9.SetPositionAllocator (positionAllocPSC9);
  mobilityPSC9.Install (psc[4].d2dPSC.Get (0));

  MobilityHelper mobilityPSC10;
  mobilityPSC10.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityPSC10.SetPositionAllocator (positionAllocPSC10);
  mobilityPSC10.Install (psc[4].d2dPSC.Get (1));

  //Commercial
  MobilityHelper mobilityComm1;
  mobilityComm1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm1.SetPositionAllocator (positionAllocComm1);
  mobilityComm1.Install (comm[0].d2dComm.Get (0));

  MobilityHelper mobilityComm2;
  mobilityComm2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm2.SetPositionAllocator (positionAllocComm2);
  mobilityComm2.Install (comm[0].d2dComm.Get (1));

  MobilityHelper mobilityComm3;
  mobilityComm3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm3.SetPositionAllocator (positionAllocComm3);
  mobilityComm3.Install (comm[1].d2dComm.Get (0));

  MobilityHelper mobilityComm4;
  mobilityComm4.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm4.SetPositionAllocator (positionAllocComm4);
  mobilityComm4.Install (comm[1].d2dComm.Get (1));

  MobilityHelper mobilityComm5;
  mobilityComm5.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm5.SetPositionAllocator (positionAllocComm5);
  mobilityComm5.Install (comm[2].d2dComm.Get (0));

  MobilityHelper mobilityComm6;
  mobilityComm6.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm6.SetPositionAllocator (positionAllocComm6);
  mobilityComm6.Install (comm[2].d2dComm.Get (1));

  MobilityHelper mobilityComm7;
  mobilityComm7.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm7.SetPositionAllocator (positionAllocComm7);
  mobilityComm7.Install (comm[3].d2dComm.Get (0));

  MobilityHelper mobilityComm8;
  mobilityComm8.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm8.SetPositionAllocator (positionAllocComm8);
  mobilityComm8.Install (comm[3].d2dComm.Get (1));

  MobilityHelper mobilityComm9;
  mobilityComm9.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm9.SetPositionAllocator (positionAllocComm9);
  mobilityComm9.Install (comm[4].d2dComm.Get (0));

  MobilityHelper mobilityComm10;
  mobilityComm10.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityComm10.SetPositionAllocator (positionAllocComm10);
  mobilityComm10.Install (comm[4].d2dComm.Get (1));

  Vector enb; 
  enb = positionAllocEnb->GetNext();


  //Install LTE devices to the nodes and fix the random number stream
  int64_t randomStream = 1;

  NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice (enbNode);
  randomStream += lteHelper->AssignStreams (enbDevs, randomStream);

  //PSC
  NetDeviceContainer psc1 = lteHelper->InstallUeDevice (psc[0].d2dPSC);
  randomStream += lteHelper->AssignStreams (psc1, randomStream);
  NetDeviceContainer psc2 = lteHelper->InstallUeDevice (psc[1].d2dPSC);
  randomStream += lteHelper->AssignStreams (psc2, randomStream);
  NetDeviceContainer psc3 = lteHelper->InstallUeDevice (psc[2].d2dPSC);
  randomStream += lteHelper->AssignStreams (psc3, randomStream);
  NetDeviceContainer psc4 = lteHelper->InstallUeDevice (psc[3].d2dPSC);
  randomStream += lteHelper->AssignStreams (psc4, randomStream);
  NetDeviceContainer psc5 = lteHelper->InstallUeDevice (psc[4].d2dPSC);
  randomStream += lteHelper->AssignStreams (psc5, randomStream);

  //Commercial
  NetDeviceContainer comm1 = lteHelper->InstallUeDevice (comm[0].d2dComm);
  randomStream += lteHelper->AssignStreams (comm1, randomStream);
  NetDeviceContainer comm2 = lteHelper->InstallUeDevice (comm[1].d2dComm);
  randomStream += lteHelper->AssignStreams (comm2, randomStream);
  NetDeviceContainer comm3 = lteHelper->InstallUeDevice (comm[2].d2dComm);
  randomStream += lteHelper->AssignStreams (comm3, randomStream);
  NetDeviceContainer comm4 = lteHelper->InstallUeDevice (comm[3].d2dComm);
  randomStream += lteHelper->AssignStreams (comm4, randomStream);
  NetDeviceContainer comm5 = lteHelper->InstallUeDevice (comm[4].d2dComm);
  randomStream += lteHelper->AssignStreams (comm5, randomStream);

  //Configure Sidelink Pool
  Ptr<LteSlEnbRrc> enbSidelinkConfiguration = CreateObject<LteSlEnbRrc> ();
  enbSidelinkConfiguration->SetSlEnabled (true); //eNb will configure the sidelink i.e. eNb will allocate resources

  //Preconfigure pool for the group
  LteRrcSap::SlCommTxResourcesSetup pool;

  pool.setup = LteRrcSap::SlCommTxResourcesSetup::SCHEDULED; //SCHEDULED for mode1 and UE_SELECTED for mode2
  //In case of SCHEDULED enb will take care of all the resource allocation

  //BSR timers
  //Based on BSR eNb will decide how much resources should be allocated to UE
  pool.scheduled.macMainConfig.periodicBsrTimer.period = LteRrcSap::PeriodicBsrTimer::sf16; //Periodic BSR
  pool.scheduled.macMainConfig.retxBsrTimer.period = LteRrcSap::RetxBsrTimer::sf640; //Regular BSR
  //MCS
  pool.scheduled.haveMcs = true;
  pool.scheduled.mcs = 16;
  //resource pool
  LteSlResourcePoolFactory pfactory;
  pfactory.SetHaveUeSelectedResourceConfig (false); //since we want eNB to schedule

  //Control
  pfactory.SetControlPeriod ("sf40");
  pfactory.SetControlBitmap (0x00000000FF); //8 subframes for PSCCH
  pfactory.SetControlOffset (0);
  pfactory.SetControlPrbNum (22);
  pfactory.SetControlPrbStart (0);
  pfactory.SetControlPrbEnd (49);

  pool.scheduled.commTxConfig = pfactory.CreatePool ();

  uint32_t groupL2Address = 255; //to which group the D2D communication belongs to i.e. Group ID

  enbSidelinkConfiguration->AddPreconfiguredDedicatedPool (groupL2Address, pool);
  lteHelper->InstallSidelinkConfiguration (enbDevs, enbSidelinkConfiguration);

  //pre-configuration for the UEs
  Ptr<LteSlUeRrc> ueSidelinkConfiguration = CreateObject<LteSlUeRrc> ();
  ueSidelinkConfiguration->SetSlEnabled (true);

  LteRrcSap::SlPreconfiguration preconfiguration;
  ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);
  lteHelper->InstallSidelinkConfiguration (psc1, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (psc2, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (psc3, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (psc4, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (psc5, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (comm1, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (comm2, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (comm3, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (comm4, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (comm5, ueSidelinkConfiguration);

  InternetStackHelper internet;
  internet.Install (psc[0].d2dPSC);
  internet.Install (psc[1].d2dPSC);
  internet.Install (psc[2].d2dPSC);
  internet.Install (psc[3].d2dPSC);
  internet.Install (psc[4].d2dPSC);
  internet.Install (comm[0].d2dComm);
  internet.Install (comm[1].d2dComm);
  internet.Install (comm[2].d2dComm);
  internet.Install (comm[3].d2dComm);
  internet.Install (comm[4].d2dComm);

  //group address / broadcast address
  Ipv4Address groupAddress4 ("225.0.0.0");     //use multicast address as destination
  Ipv6Address groupAddress6 ("ff0e::1");     //use multicast address as destination
  Address remoteAddress;
  Address localAddress;
  Ptr<LteSlTft> tft;
  if (!useIPv6)
    {
      //Install the IP stack on the UEs and assign IP address
      Ipv4InterfaceContainer pscIpIface1;
      pscIpIface1 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (psc1));

      Ipv4InterfaceContainer pscIpIface2;
      pscIpIface2 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (psc2));

      Ipv4InterfaceContainer pscIpIface3;
      pscIpIface3 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (psc3));

      Ipv4InterfaceContainer pscIpIface4;
      pscIpIface4 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (psc4));

      Ipv4InterfaceContainer pscIpIface5;
      pscIpIface5 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (psc5));

      Ipv4InterfaceContainer commIpIface1;
      commIpIface1 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (comm1));

      Ipv4InterfaceContainer commIpIface2;
      commIpIface2 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (comm2));

      Ipv4InterfaceContainer commIpIface3;
      commIpIface3 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (comm3));

      Ipv4InterfaceContainer commIpIface4;
      commIpIface4 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (comm4));

      Ipv4InterfaceContainer commIpIface5;
      commIpIface5 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (comm5));

      // set the default gateway for the UE
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      for (uint32_t u = 0; u < psc[0].d2dPSC.GetN (); ++u)
        {
          Ptr<Node> ueNode = psc[0].d2dPSC.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      for (uint32_t u = 0; u < psc[1].d2dPSC.GetN (); ++u)
        {
          Ptr<Node> ueNode = psc[1].d2dPSC.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      for (uint32_t u = 0; u < psc[2].d2dPSC.GetN (); ++u)
        {
          Ptr<Node> ueNode = psc[2].d2dPSC.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      for (uint32_t u = 0; u < psc[3].d2dPSC.GetN (); ++u)
        {
          Ptr<Node> ueNode = psc[3].d2dPSC.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      for (uint32_t u = 0; u < psc[4].d2dPSC.GetN (); ++u)
        {
          Ptr<Node> ueNode = psc[4].d2dPSC.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      for (uint32_t u = 0; u < comm[0].d2dComm.GetN (); ++u)
        {
          Ptr<Node> ueNode = comm[0].d2dComm.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      for (uint32_t u = 0; u < comm[1].d2dComm.GetN (); ++u)
        {
          Ptr<Node> ueNode = comm[1].d2dComm.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      for (uint32_t u = 0; u < comm[2].d2dComm.GetN (); ++u)
        {
          Ptr<Node> ueNode = comm[2].d2dComm.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      for (uint32_t u = 0; u < comm[3].d2dComm.GetN (); ++u)
        {
          Ptr<Node> ueNode = comm[3].d2dComm.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      for (uint32_t u = 0; u < comm[4].d2dComm.GetN (); ++u)
        {
          Ptr<Node> ueNode = comm[4].d2dComm.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }

      remoteAddress = InetSocketAddress (groupAddress4, 8000);
      localAddress = InetSocketAddress (Ipv4Address::GetAny (), 8000);
      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress4, groupL2Address);
    }
  else
    {
      //We are using IPv4 for this project
      /*
      Ipv6InterfaceContainer ueIpIface;
      ueIpIface = epcHelper->AssignUeIpv6Address (NetDeviceContainer (ueDevs));

      // set the default gateway for the UE
      Ipv6StaticRoutingHelper Ipv6RoutingHelper;
      for (uint32_t u = 0; u < psc[0].d2dPSC.GetN (); ++u)
        {
          Ptr<Node> ueNode = psc[0].d2dPSC.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv6StaticRouting> ueStaticRouting = Ipv6RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv6> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress6 (), 1);
        }
      remoteAddress = Inet6SocketAddress (groupAddress6, 8000);
      localAddress = Inet6SocketAddress (Ipv6Address::GetAny (), 8000);
      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress6, groupL2Address);
      */
    }

  //Attach each UE to the best available eNB
  lteHelper->Attach (psc1, enbDevs.Get (0));
  lteHelper->Attach (psc2, enbDevs.Get (0));
  lteHelper->Attach (psc3, enbDevs.Get (0));
  lteHelper->Attach (psc4, enbDevs.Get (0));
  lteHelper->Attach (psc5, enbDevs.Get (0));
  lteHelper->Attach (comm1, enbDevs.Get (0));
  lteHelper->Attach (comm2, enbDevs.Get (0));
  lteHelper->Attach (comm3, enbDevs.Get (0));
  lteHelper->Attach (comm4, enbDevs.Get (0));
  lteHelper->Attach (comm5, enbDevs.Get (0));

  ///*** Configure applications ***///

  //Set Application in the UEs
  OnOffHelper sidelinkClient ("ns3::UdpSocketFactory", remoteAddress);
  sidelinkClient.SetConstantRate (DataRate ("16kb/s"), 300);  //Packet size 300

  //Create application containers for both PSC and Commercial Client and Servers
  ApplicationContainer clientAppsPSC;
  ApplicationContainer serverAppsPSC;
  ApplicationContainer clientAppsComm;
  ApplicationContainer serverAppsComm;
  int t=0; 
  
  //Both PSC and Commercial applications will run in parallel
  for(int i=0; i<5; i++)
  {
     if(psc[i].application_type == 1)  //PSC
     { 
      
      NS_LOG_UNCOND("PSC application Client server creation");
      ApplicationContainer clientApps1 = sidelinkClient.Install (psc[i].d2dPSC.Get (0));
      clientApps1.Start (Seconds (t + 1.9));
      clientApps1.Stop (Seconds (11.0));
      clientAppsPSC.Add(clientApps1);
      
      ApplicationContainer serverApps1;
      PacketSinkHelper sidelinkSink1 ("ns3::UdpSocketFactory", localAddress);
      serverApps1 = sidelinkSink1.Install (psc[i].d2dPSC.Get (1));
      serverApps1.Start (Seconds (t+0.9));
      serverAppsPSC.Add(serverApps1);
      
      t++;
     }

     if(comm[i].application_type == 2)  //Commercial
     { 
      
      NS_LOG_UNCOND("Commercial application Client server creation");
      ApplicationContainer clientApps1 = sidelinkClient.Install (comm[i].d2dComm.Get (0));
      clientApps1.Start (Seconds (t + 1.9));
      clientApps1.Stop (Seconds (11.0));
      clientAppsComm.Add(clientApps1);
      
      ApplicationContainer serverApps1;
      PacketSinkHelper sidelinkSink1 ("ns3::UdpSocketFactory", localAddress);
      serverApps1 = sidelinkSink1.Install (comm[i].d2dComm.Get (1));
      serverApps1.Start (Seconds (t+0.9));
      serverAppsComm.Add(serverApps1);
      
      t++;
     }
  }

  //Activating Sidelink Bearers for all UEs
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, psc1, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, psc2, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, psc3, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, psc4, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, psc5, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, comm1, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, comm2, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, comm3, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, comm4, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, comm5, tft);
  ///*** End of application configuration ***///

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("ACN_Project.tr"); //Trace file

  //Trace file table header
  *stream->GetStream () << "time(sec)\ttx/rx\tNodeID\tIMSI\tPktSize(bytes)\tIP[src]\tIP[dst]" << std::endl;

  std::ostringstream oss;

  
  if (!useIPv6)
    {
      // Set Tx traces
      for (uint16_t ac = 0; ac < clientAppsPSC.GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientAppsPSC.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Tx address: " << localAddrs << std::endl;
          oss << "tx\t" << psc[ac].d2dPSC.Get (0)->GetId () << "\t" << psc[ac].d2dPSC.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientAppsPSC.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }

        for (uint16_t ac = 0; ac < clientAppsComm.GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientAppsComm.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Tx address: " << localAddrs << std::endl;
          oss << "tx\t" << comm[ac].d2dComm.Get (0)->GetId () << "\t" << comm[ac].d2dComm.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientAppsComm.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }

      // Set Rx traces
      for (uint16_t ac = 0; ac < serverAppsPSC.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverAppsPSC.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          oss << "rx\t" << psc[ac].d2dPSC.Get (1)->GetId () << "\t" << psc[ac].d2dPSC.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverAppsPSC.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }

        for (uint16_t ac = 0; ac < serverAppsComm.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverAppsComm.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          oss << "rx\t" << comm[ac].d2dComm.Get (1)->GetId () << "\t" << comm[ac].d2dComm.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverAppsComm.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }
    }
  else{
        for (uint16_t ac = 0; ac < clientAppsPSC.GetN (); ac++)
        {
              Ipv4Address localAddrs =  clientAppsPSC.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
              std::cout << "Tx address: " << localAddrs << std::endl;
              oss << "tx\t" << psc[ac].d2dPSC.Get (0)->GetId () << "\t" << psc[ac].d2dPSC.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
              clientAppsPSC.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
              oss.str ("");
        }

        for (uint16_t ac = 0; ac < clientAppsComm.GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientAppsComm.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Tx address: " << localAddrs << std::endl;
          oss << "tx\t" << comm[ac].d2dComm.Get (0)->GetId () << "\t" << comm[ac].d2dComm.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientAppsComm.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }

      // Set Rx traces
      for (uint16_t ac = 0; ac < serverAppsPSC.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverAppsPSC.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          oss << "rx\t" << psc[ac].d2dPSC.Get (1)->GetId () << "\t" << psc[ac].d2dPSC.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverAppsPSC.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }

        for (uint16_t ac = 0; ac < serverAppsComm.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverAppsComm.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          oss << "rx\t" << comm[ac].d2dComm.Get (1)->GetId () << "\t" << comm[ac].d2dComm.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverAppsComm.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }
  }

  NS_LOG_UNCOND ("Enabling Sidelink traces...");
  lteHelper->EnableSidelinkTraces ();

  NS_LOG_UNCOND ("Starting simulation...");

  //FlowMonitor used for finding Throughput
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> allMon = fmHelper.InstallAll();
  Simulator::Schedule(Seconds(3),&ThroughputMonitor,&fmHelper, allMon);

  Simulator::Stop (simTime);
    
  //Setting up NetAnim
  AnimationInterface anim (animFile);
  anim.EnablePacketMetadata (); 
  anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (10)); 
   
  //Calling the function for finding SINR
  Config::Connect ("/NodeList/*/DeviceList/0/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr", MakeCallback (&PrintSinrTrace));

  Simulator::Run ();
  ThroughputMonitor(&fmHelper, allMon); //Function for finding Throughput 
  //std::cout<<"Average SINR : "<<avg_sinr<<", No. of SINR :"<<no_sinrs<<std::endl; 
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  std::cout<<"Average SINR : "<<avg_sinr/no_sinrs<<std::endl;  //Average SINR
  std::cout<<"End to end Delay : "<<Delay<<std::endl;

  //A step for executing the bash file
  FILE *fptr;
  fptr = fopen("./scratch/sinr.txt","w");

  fprintf(fptr,"%f",avg_sinr/no_sinrs);
  fclose(fptr);
  
  Simulator::Destroy ();
  return 0;

}

//Function for finding Throughput using FlowMontior
void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon){

    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
    {
      Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
      std::cout<<"Flow ID     : " << stats->first<<std::endl;
      std::cout<<"Source Address : "<< fiveTuple.sourceAddress <<" -----> Destination Address : "<<fiveTuple.destinationAddress<<std::endl;
      std::cout<<"Delay : "<<stats->second.delaySum<<std::endl;
      std::cout<<"Jitter : "<<stats->second.jitterSum<< std::endl;
      std::cout<<"Duration    : "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<std::endl;
      std::cout<<"Last Received Packet  : "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
      std::cout<<"Throughput: " << stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps"<<std::endl;
      std::cout<<"---------------------------------------------------------------------------"<<std::endl;

      Delay = Delay + (stats->second.delaySum);
      Jitter = Jitter + (stats->second.jitterSum);
    }

    Simulator::Schedule(Seconds(1),&ThroughputMonitor, fmhelper, flowMon);
}