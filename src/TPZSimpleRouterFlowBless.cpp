//
//   Copyright (C) 1998-2011 by Galerna Project, the University of
//   Cantabria, Spain.
//
//   This file is part of the TOPAZ network simulator, originallty developed
//   at the Unviersity of Cantabria
//
//   TOPAZ shares a large proportion of code with SICOSYS which was
//   developed by V.Puente and J.M.Prellezo
//
//   TOPAZ has been developed by P.Abad, L.G.Menezo, P.Prieto and
//   V.Puente
//
//  --------------------------------------------------------------------
//
//  If your use of this software contributes to a published paper, we
//  request that you (1) cite our summary paper that appears on our
//  website (http://www.atc.unican.es/topaz/) and (2) e-mail a citation
//  for your published paper to topaz@atc.unican.es
//
//  If you redistribute derivatives of this software, we request that
//  you notify us and either (1) ask people to register with us at our
//  website (http://www.atc.unican.es/topaz/) or (2) collect registration
//  information and periodically send it to us.
//
//   --------------------------------------------------------------------
//
//   TOPAZ is free software; you can redistribute it and/or
//   modify it under the terms of version 2 of the GNU General Public
//   License as published by the Free Software Foundation.
//
//   TOPAZ is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with the TOPAZ simulator; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//   02111-1307, USA
//
//   The GNU General Public License is contained in the file LICENSE.
//
//
//*************************************************************************
//:
//    File: TPZSimpleRouterFlowBless.cpp
//
//    Class:  TPZSimpleRouterFlowBless
//
//    Inherited from:  TPZFlow
// :
//*************************************************************************
//end of header

#include <TPZSimpleRouterFlowBless.hpp>

#ifndef __TPZInterfaz_HPP__
#include <TPZInterfaz.hpp>
#endif

#ifndef __TPZFicheroLog_HPP__
#include <TPZFicheroLog.hpp>
#endif

#ifndef __TPZSimulation_HPP__
#include <TPZSimulation.hpp>
#endif

#ifndef __TPZRouter_HPP__
#include <TPZRouter.hpp>
#endif

#ifndef __TPZSimpleRouter_HPP__
#include <TPZSimpleRouter.hpp>
#endif

//#define DEBUG

/// CHIPPER Modified Files //
/*
   TPZMessage.hpp
   TPZMessage.cpp
   TPZNetworkTorus.hpp
   TPZNetworkTorus.cpp
   TPZSimpleRouterFlowBless.hpp
   TPZSimpleRouterFlowBless.cpp
   TPZSimulation.hpp
   TPZSimulation.cpp
   TPZNetwork.cpp
*/



//*************************************************************************

IMPLEMENT_RTTI_DERIVED(TPZSimpleRouterFlowBless,TPZFlow);

//*************************************************************************
//:
//  f: TPZSimpleRouterFlow();
//
//  d:
//:
//*************************************************************************

TPZSimpleRouterFlowBless :: TPZSimpleRouterFlowBless( TPZComponent& component)
                 : TPZFlow(component)
{
}


//*************************************************************************
//:
//  f: ~TPZSimpleRouterFlow();
//
//  d:
//:
//*************************************************************************

TPZSimpleRouterFlowBless :: ~TPZSimpleRouterFlowBless()
{
}


//*************************************************************************
//:
//  f: virtual void initialize();
//
//  d:
//:
//*************************************************************************
extern unsigned long maxInject;
extern unsigned networkSize;
unsigned routerDelay = 2;
//unsigned Epoch = networkSize*routerDelay + networkSize;
//unsigned Epoch = networkSize*3 + networkSize;
void TPZSimpleRouterFlowBless :: initialize()
{
   Inhereited :: initialize();

   m_ports=((TPZSimpleRouter&)getComponent()).numberOfInputs();
   m_connections=new unsigned[m_ports+1];
   m_sync=new TPZMessage*[m_ports+1];
   m_pipeReg1 = new TPZMessage*[m_ports];
//   m_pipeReg2 = new TPZMessage*[m_ports];
   m_eject = new Boolean [m_ports];
   m_connEstablished=new Boolean[m_ports+1];
   pktIdGolden = 1;
   m_interFlit = new TPZMessage * [m_ports];
   m_outFlit = new TPZMessage * [m_ports];

   for(int i=0; i<m_ports+1; i++)
   {
      m_connections[i] = 0;
      m_connEstablished[i] = false;
      m_sync[i]=0;
   }

   for(int i=0; i<m_ports; i++)
   {
      m_eject[i] = false;
      m_pipeReg1[i] = 0;
 //     m_pipeReg2[i] = 0;
      m_interFlit[i] = 0;
      m_outFlit[i] = 0;
   }

   setCleanInterfaces(true);
}


//*************************************************************************
//:
//  f: virtual void terminate();
//
//  d:
//:
//*************************************************************************

void TPZSimpleRouterFlowBless :: terminate()
{
   delete[] m_connections;
   delete[] m_sync;
   delete[] m_connEstablished;
   delete[] m_pipeReg1;
//   delete[] m_pipeReg2;
   delete[] m_eject;
   delete[] m_interFlit;
   delete[] m_outFlit;
}





//*************************************************************************
//:
//  f: virtual Boolean inputReading();
//
//  d:
//:
//*************************************************************************
Boolean TPZSimpleRouterFlowBless :: inputReading()
{
   #ifdef DEBUG
   cout << endl << endl << "TIME = ";
   TPZString texto = TPZString(getOwnerRouter().getCurrentTime()) + " @ " + getComponent().asString();
   cout << texto << endl;
   #endif // DEBUG

   unsigned outPort;
   unsigned inPort;

   //**********************************************************************************************************
   // PART3: Permutation Network
   //**********************************************************************************************************

   bool swapEnable [4] = {};
   cleanOutputInterfaces();

   swapEnable[0] = permuterBlock (m_pipeReg1[4], m_pipeReg1[2], 1);
   swapEnable[1] = permuterBlock (m_pipeReg1[3], m_pipeReg1[1], 1);
   if (swapEnable[0]==true)
   {
      m_interFlit[1] = m_pipeReg1[2];
      m_interFlit[2] = m_pipeReg1[4];
   }
   else
   {
      m_interFlit[1] = m_pipeReg1[4];
      m_interFlit[2] = m_pipeReg1[2];
   }
   if (swapEnable[1]==true)
   {
      m_interFlit[3] = m_pipeReg1[1];
      m_interFlit[4] = m_pipeReg1[3];
   }
   else
   {
      m_interFlit[3] = m_pipeReg1[3];
      m_interFlit[4] = m_pipeReg1[1];
   }
//   swapFlit (m_pipeReg1[0],m_pipeReg1[1],&(m_interFlit[0]),&(m_interFlit[1]),swapEnable[0]);
//   swapFlit (m_pipeReg1[2],m_pipeReg1[3],&m_interFlit[2],&m_interFlit[3],swapEnable[1]);
   for (inPort=1; inPort<m_ports; inPort++)
      m_pipeReg1[inPort] = 0;

   swapEnable[2] = permuterBlock (m_interFlit[1], m_interFlit[3], 2);
   swapEnable[3] = permuterBlock (m_interFlit[2], m_interFlit[4], 2);
//   swapFlit (m_interFlit[0],m_interFlit[2],m_outFlit[0],m_outFlit[1],swapEnable[2]);
//   swapFlit (m_interFlit[1],m_interFlit[3],m_outFlit[2],m_outFlit[3],swapEnable[3]);
   if (swapEnable[2]==true)
   {
      m_outFlit[1] = m_interFlit[3];
      m_outFlit[2] = m_interFlit[1];
   }
   else
   {
      m_outFlit[1] = m_interFlit[1];
      m_outFlit[2] = m_interFlit[3];
   }
   if (swapEnable[3]==true)
   {
      m_outFlit[3] = m_interFlit[4];
      m_outFlit[4] = m_interFlit[2];
   }
   else
   {
      m_outFlit[3] = m_interFlit[2];
      m_outFlit[4] = m_interFlit[4];
   }

   for (inPort=1; inPort<m_ports; inPort++)
      m_interFlit[inPort] = 0;

   for (outPort=1; outPort<m_ports; outPort++)
   {
      if (m_outFlit[outPort])
      {
         m_outFlit[outPort]->clearGolden(); // calculate at each node.
         /// m_outFlit is directly connected to outPorts.
         /*
            Direction   outPort   m_outFlit
            East        1           3
            West        2           4
            North       3           1
            South       4           2
         */
         switch (outPort)
         {
            case 1:   
                     if (m_outFlit[outPort]->getRoutingPort() != _Yplus_)
                        ((TPZNetwork*)(getOwnerRouter().getOwner()))->incrEventCount(TPZNetwork::Deflection);
                     outputInterfaz(3)->sendData(m_outFlit[1]); // Send to North
                     break;
                   
            case 2:   
                     if (m_outFlit[outPort]->getRoutingPort() != _Yminus_)
                        ((TPZNetwork*)(getOwnerRouter().getOwner()))->incrEventCount(TPZNetwork::Deflection);
                     outputInterfaz(4)->sendData(m_outFlit[2]); // Send to South
                        break;
            case 3:  
                     if (m_outFlit[outPort]->getRoutingPort() != _Xplus_)
                        ((TPZNetwork*)(getOwnerRouter().getOwner()))->incrEventCount(TPZNetwork::Deflection);
                     outputInterfaz(1)->sendData(m_outFlit[3]); // Send to East
                        break;
            case 4:   
                     if (m_outFlit[outPort]->getRoutingPort() != _Xminus_)
                        ((TPZNetwork*)(getOwnerRouter().getOwner()))->incrEventCount(TPZNetwork::Deflection);
                     outputInterfaz(2)->sendData(m_outFlit[4]); // Send to West
                        break;
         }
         
         // Determine if deflection occur
         //if (outPort != m_outFlit[outPort]->getRoutingPort())
         m_outFlit[outPort] = 0;
         ((TPZNetwork*)(getOwnerRouter().getOwner()))->incrEventCount( TPZNetwork::SWTraversal);
         //((TPZNetwork*)(getOwnerRouter().getOwner()))->incrEventCount( TPZNetwork::LinkTraversal);
         //getOwnerRouter().incrLinkUtilization();

      }
   }



   //**********************************************************************************************************
   // PART1: Compute Delta Info and Set Golden Bit
   //**********************************************************************************************************

   goldenEpochSequencing (getOwnerRouter().getCurrentTime());

   for( inPort = 1; inPort < m_ports; inPort++)
   {
      if(m_sync[inPort])
      {
         #ifdef DEBUG
         cout << "Flit arrive at inPort = " << inPort << endl;
         cout << "        PktID = " << m_sync[inPort]->getIdentifier() << endl;
         cout << "        Flit " << m_sync[inPort]->flitNumber() << "/" << m_sync[inPort]->getPacketLength() << endl;
         cout << "        Dest = " << m_sync[inPort]->destiny().asString() << endl;
         #endif // DEBUG

         routeComputation(m_sync[inPort]);
         for (outPort = 1; outPort <= m_ports; outPort++)
         {
            if(getDeltaAbs(m_sync [inPort], outPort)==true)
            {
               m_sync [inPort]->setRoutingPort((TPZROUTINGTYPE) outPort);
               break;
            }
         }
         if (goldenCheck (m_sync[inPort]->getIdentifier()) == true)
            m_sync[inPort]->setGolden();
         else
            m_sync[inPort]->clearGolden();
         m_sync[inPort]->setInputInterfaz(inPort);
         m_pipeReg1 [inPort] = m_sync[inPort];
         m_sync[inPort]=0;
         #ifdef DEBUG
         cout << "Flit successfully stored in pipeReg1 = " << inPort << endl;
         cout << "        PktID = " << m_pipeReg1[inPort]->getIdentifier() << endl;
         cout << "        Flit " << m_pipeReg1[inPort]->flitNumber() << "/" << m_pipeReg1[inPort]->getPacketLength() << endl;
         cout << "        Dest = " << m_pipeReg1[inPort]->destiny().asString() << endl;
         if (m_pipeReg1[inPort]->getGolden())
            cout << "        This flit is golden." << endl;
         else
            cout << "        This flit is not golden." << endl;
         #endif // DEBUG
      }
   }

    //**********************************************************************************************************
   //  Eject - Ejection Kill - Injection
   //**********************************************************************************************************

   for( inPort = 1; inPort < m_ports; inPort++)
   {
      m_eject[inPort] = false; // reset it.
      if (m_pipeReg1 [inPort])
         if(m_pipeReg1[inPort] -> getRoutingPort() == _LocalNode_)
            m_eject[inPort] = true;
   }
   TPZMessage * msgEject;
   msgEject = ejector ();
   if (msgEject)
   {
      outputInterfaz(m_ports)->sendData(msgEject);
      m_pipeReg1 [msgEject->getInputInterfaz()] = 0; // ejector kill to remove the ejected msg.
   }

   //    check the local injection and put the flit into injection queue.
   //    The injection queue should reside in NI.
   //*********************************************************************************************************
   if(m_sync[m_ports])
   {
      #ifdef DEBUG
      cout<< "Flit " << m_sync[m_ports]->flitNumber() << "/" << m_sync[m_ports]->getPacketLength() <<" of Pkt " << m_sync[m_ports]->getIdentifier() << " at NI OutPort" << endl;
      #endif // DEBUG
      m_injectionQueue.enqueue(m_sync[m_ports]);
      m_sync[inPort]=0;
   }

   if(m_injectionQueue.numberOfElements() !=0)
   {
      for ( inPort = 1; inPort < m_ports; inPort++)
      {
         if(m_pipeReg1[inPort] == 0)
         {
            m_injectionQueue.dequeue(m_pipeReg1[inPort]); // Anderson: construct a outstanding msg.
            #ifdef DEBUG
            cout<< "Flit " << m_pipeReg1[inPort]->flitNumber() << "/" << m_pipeReg1[inPort]->getPacketLength() <<" of Pkt "\
             << m_pipeReg1[inPort]->getIdentifier() << " is injected to inPort =" << inPort << endl;
            #endif // DEBUG

            routeComputation(m_pipeReg1[inPort]);
            for (outPort = 1; outPort < m_ports; outPort++) // no need to check local outPort
            {
               if(getDeltaAbs(m_pipeReg1[inPort], outPort)==true)
               {
                  m_pipeReg1 [inPort]->setRoutingPort((TPZROUTINGTYPE) outPort);
                  #ifdef DEBUG
                  cout << "    Desired output port = " << outPort << endl;
                  #endif // DEBUG
                  break;
               }
            }
            if (goldenCheck (m_pipeReg1[inPort]->getIdentifier()) == true)
            {
               m_pipeReg1[inPort]->setGolden();
               #ifdef DEBUG
               cout << "    This flit is golden." << endl;
               #endif // DEBUG
            }
            else
            {
               m_pipeReg1[inPort]->clearGolden();
               #ifdef DEBUG
               cout << "    This flit is NOT golden." << endl;
               #endif // DEBUG
            }
            m_pipeReg1[inPort]->setInputInterfaz(inPort);
            break;
         }
      }
   }

   // if the injection condition is not satisfied, need to stop injection by sending STOP to the local interfaz.
   if (m_injectionQueue.numberOfElements() !=0) inputInterfaz(m_ports)->sendStopRightNow();
   else inputInterfaz(m_ports)->clearStopRightNow();
   /*
   for ( inPort = 1; inPort < m_ports; inPort++)
   {
      if (m_pipeReg1[inPort] != 0)
      {
         m_pipeReg2[inPort] = m_pipeReg1[inPort];
         m_pipeReg1[inPort] = 0;
      }
   }
   */
   return true;
}











void TPZSimpleRouterFlowBless :: goldenEpochSequencing (unsigned time)
{
   unsigned Epoch = networkSize*routerDelay + 4;
   // just to skip the first clock cycle.
   if ((time - 1) % Epoch == 0 && (time > 1)) // change to if statement when connecting to TOPAZ
   {
       if (pktIdGolden == maxInject)
         pktIdGolden = 1;
      else
         pktIdGolden ++;
   }
}

Boolean TPZSimpleRouterFlowBless :: goldenCheck (unsigned pktId)
{
   // In paper, "it is not to check pkt at each hop, however, every pkt has to be check at least once in each epoch"
   // There are two options
   // 1.at the beginning of each epoch, check all pkts in the network. then use 1-bit to carry the result
   // 2.check at each hop every cycle. no need to carry result.
   // I use 2;

   if (pktId == pktIdGolden)
      return true;
   else
      return false;

}


unsigned TPZSimpleRouterFlowBless :: steering (TPZROUTINGTYPE desiredDirection, unsigned stage)
{
   unsigned desiredPort = 0; // desired outPort in each permuter block.

   if (stage == 1)
   {
      // steering function for stage 1
      if (desiredDirection == _Yplus_ || desiredDirection == _Yminus_)
         desiredPort = 0;
      else if (desiredDirection == _Xplus_ || desiredDirection == _Xminus_)
         desiredPort = 1;
   }
   else if (stage == 2)
   {
      // steering function for stage 2
      if (desiredDirection == _Yplus_ || desiredDirection == _Xplus_)
         desiredPort = 0;
      else if (desiredDirection == _Yminus_ || desiredDirection == _Xminus_)
         desiredPort = 1;
   }
   else
      cout << "Error: wrong value! beyond the scope!" << endl;

   return desiredPort;
}

TPZMessage * TPZSimpleRouterFlowBless :: winner (TPZMessage * msgCh0, TPZMessage * msgCh1)
{
   TPZMessage * winningFlit;
   Boolean GP0, GP1;
   unsigned n0, n1;

   if(msgCh0)
   {
      GP0 = msgCh0 -> getGolden();
      n0 = msgCh0 -> flitNumber();
   }
   else
      GP0 = false;

   if(msgCh1)
   {
      n1 = msgCh1 -> flitNumber();
      GP1 = msgCh1 -> getGolden();
   }
   else
      GP1 = false;

   if (GP0 == true && GP1 == true)
      winningFlit = (n1<n0) ? msgCh1 : msgCh0;
   else if (GP0 == true)
      winningFlit = msgCh0;
   else if (GP1 == true)
      winningFlit = msgCh1;
   else
   {
      if (msgCh1==0 && msgCh0!=0)
         winningFlit = msgCh0;
      else if (msgCh1!=0 && msgCh0==0)
         winningFlit = msgCh1;
      else // both not empty, choose randomly.
      {
         if (rand() % 2)
            winningFlit = msgCh1; // pseudorandom, generate 0 or 1;
         else
            winningFlit = msgCh0;
         /// most of time, winner will be decided pseudo randomly. It will increase deflection rate, although it is cheap. Can I do better with low cost?

      }
   }

   return winningFlit;
}

bool TPZSimpleRouterFlowBless :: swapFlitCtrl (unsigned flit0DesiredPort, unsigned flit1DesiredPort, unsigned winningChannel)
{
   bool swapEnable = false;

   if ((winningChannel == 0 && flit0DesiredPort == 1) || (winningChannel == 1 && flit1DesiredPort == 0))
      swapEnable = true;

   return swapEnable;
}

bool TPZSimpleRouterFlowBless :: permuterBlock (TPZMessage * msg0, TPZMessage * msg1, unsigned stage)
{

   unsigned winningChannel;
   unsigned flit0DesiredPort = 0;
   unsigned flit1DesiredPort = 1;
   bool swapEnable = false;

   // No msg arrives
   if (msg0==0 && msg1==0)
      return false;

   if (winner (msg0,msg1) == msg0)
      winningChannel = 0;
   else if (winner (msg0,msg1) == msg1)
      winningChannel = 1;

   if (msg0)
      flit0DesiredPort = steering (msg0->getRoutingPort(), stage);
   if (msg1)
      flit1DesiredPort = steering (msg1->getRoutingPort(), stage);

   swapEnable = swapFlitCtrl (flit0DesiredPort, flit1DesiredPort, winningChannel);

   return swapEnable;
}
/*
void TPZSimpleRouterFlowBless :: swapFlit (TPZMessage * inFlit0, TPZMessage* inFlit1, TPZMessage* & outFlit0, TPZMessage *& outFlit1, bool swapEnable)
{
   if (swapEnable == true)
   {
      outFlit0 = inFlit1;
      outFlit1 = inFlit0;
   }
   else
   {
      outFlit0 = inFlit0;
      outFlit1 = inFlit1;
   }
}
*/
TPZMessage * TPZSimpleRouterFlowBless :: ejector ()
{
   TPZMessage * msg1x0;
   TPZMessage * msg1x1;
   if (m_eject[1] == true && m_eject[2] == true)
      msg1x0 = winner(m_pipeReg1[1],m_pipeReg1[2]);
   else if (m_eject[1] == true)
      msg1x0 = m_pipeReg1[1];
   else if (m_eject[2] == true)
      msg1x0 = m_pipeReg1[2];
   else
      msg1x0 = 0;

   if (m_eject[3] == true && m_eject[4] == true)
      msg1x1 = winner(m_pipeReg1[3],m_pipeReg1[4]);
   else if (m_eject[3] == true)
      msg1x1 = m_pipeReg1[3];
   else if (m_eject[4] == true)
      msg1x1 = m_pipeReg1[4];
   else
      msg1x1 = 0;

   TPZMessage * msgEject;
   if (msg1x0 != 0 && msg1x1 != 0)
      msgEject = winner(msg1x0,msg1x1);
   else if (msg1x0 != 0)
      msgEject = msg1x0;
   else if (msg1x1 != 0)
      msgEject = msg1x1;
   else
      msgEject = 0;

   return msgEject;

}

Boolean TPZSimpleRouterFlowBless :: routeComputation(TPZMessage* msg)
{
   int deltaX;
   int deltaY;
   int deltaZ;

   TPZPosition source= getOwnerRouter().getPosition();
   TPZPosition destination=msg->destiny();
   // A: This is the actual routing.
   //    routingRecord () will be overloaded based on the type of network.
   ((TPZNetwork*)getOwnerRouter().getOwner())->routingRecord(source,destination,deltaX,deltaY,deltaZ);
   // A: Header manipulation.
   msg->setDelta(deltaX,0);
   msg->setDelta(deltaY,1);
   msg->setDelta(deltaZ,2);
   return true;
}

//*************************************************************************
//:
//  f: unsigned getDeltaAbs(TPZMessage* msg, unsigned outPort);
//
//  d: // A: check if "outPort" is the desired output port for "msg".
       // Return "true" is yes.
//*************************************************************************
Boolean TPZSimpleRouterFlowBless :: getDeltaAbs(TPZMessage* msg, unsigned outPort)
{
   //the order in sgml file must fit the same one as the selected for this file
   //this means: 1-(x+) 2-(x-) 3-(y+) 4-(y-) 5-(Node)
   int deltaX=msg->delta(0);
   int deltaY=msg->delta(1);
   int deltaZ=msg->delta(2);
#ifndef NO_TRAZA
   TPZString texto4 = getComponent().asString() + " Checking Delta = ";
   texto4 += TPZString(deltaX) + " " + TPZString(deltaY) + " " + TPZString(deltaZ) + " "  + msg->asString() ;
   texto4 += TPZString(((TPZSimpleRouter&)getComponent()).getTypeForOutput(outPort));
   TPZWRITE2LOG(texto4);
#endif
   // A: not sure if tipo is a function of outPort.
   TPZROUTINGTYPE tipo=((TPZSimpleRouter&)getComponent()).getTypeForOutput(outPort);

   // A: check if "outPort" is the desired output port.
   // Return "true" is yes.
   switch(tipo)
   {
      case _Xplus_:
         if (deltaX>1) return true;
	     else return false;
	     break;
      case _Xminus_:
         if (deltaX<-1) return true;
	     else return false;
	     break;
      case _Yplus_:
         if (deltaY>1) return true;
	     else return false;
	     break;
      case _Yminus_:
         if (deltaY<-1) return true;
	     else return false;
	     break;
      case _Zplus_:
         if (deltaZ>1) return true;
	     else return false;
	     break;
      case _Zminus_:
         if (deltaZ<-1) return true;
	     else return false;
	     break;
      case _LocalNode_:
         if ( (deltaX==1 || deltaX==-1) && (deltaY==1 || deltaY==-1) && (deltaZ==1 || deltaZ==-1) ) return true;
	     else return false;
	     break;

      default:
         TPZString err;
	     err.sprintf("%s :output port out of range", (char*)getComponent().asString() );
	     EXIT_PROGRAM(err);
	 break;
   }

}

//*************************************************************************
//:
//  f: virtual Boolean updateMessageInfo(TPZMessage& msg, unsigned outPort);
//
//  d: (By Anderson) update port allocation result
//:
//*************************************************************************

Boolean TPZSimpleRouterFlowBless :: updateMessageInfo(TPZMessage* msg, unsigned outPort)
{
   int deltaX;
   int deltaY;
   int deltaZ;

   // A: get the network size
   unsigned SizeX= ((TPZNetwork*)getOwnerRouter().getOwner())->getSizeX();
   unsigned SizeY= ((TPZNetwork*)getOwnerRouter().getOwner())->getSizeY();
   unsigned SizeZ= ((TPZNetwork*)getOwnerRouter().getOwner())->getSizeZ();

   // A: get the position of the current node.
   //    source seems to be updated each hop.
   TPZPosition source= getOwnerRouter().getPosition();
   int PosX=source.valueForCoordinate(TPZPosition::X);
   int PosY=source.valueForCoordinate(TPZPosition::Y);
   int PosZ=source.valueForCoordinate(TPZPosition::Z);

   // A: get the destination of the msg.
   TPZPosition destination=msg->destiny();

   TPZROUTINGTYPE tipo=((TPZSimpleRouter&)getComponent()).getTypeForOutput(outPort);

   // A: update the position of next hop based on the routing algorithm
   switch(tipo)
   {
      case _Xplus_:
         PosX= (PosX+1)%SizeX;
	     msg->setRoutingPort(_Xplus_);
	     break;
      case _Xminus_:
         PosX= (PosX+(SizeX-1))%SizeX;
	     msg->setRoutingPort(_Xminus_);
	     break;
      case _Yplus_:
         PosY= (PosY+1)%SizeY;
         msg->setRoutingPort(_Yplus_);
	     break;
      case _Yminus_:
         PosY=(PosY+(SizeY-1))%SizeY;
         msg->setRoutingPort(_Yminus_);
	     break;
      case _Zplus_:
         PosZ= (PosZ+1)%SizeZ;
         msg->setRoutingPort(_Zplus_);
	     break;
      case _Zminus_:
         PosZ=(PosZ+(SizeZ-1))%SizeZ;
         msg->setRoutingPort(_Zminus_);
	     break;
      case _LocalNode_:
         msg->setRoutingPort(_LocalNode_);
	     break;
      default:
         TPZString err;
	     err.sprintf("%s :output port out of range", (char*)getComponent().asString() );
	     EXIT_PROGRAM(err);
	     break;
   }
   // A: source information is updated.
   source.setValueAt(TPZPosition::X, PosX);
   source.setValueAt(TPZPosition::Y, PosY);
   source.setValueAt(TPZPosition::Z, PosZ);

   // A: This might be the actual routing.
   //    routingRecord () will be overloaded based on the type of network.
   ((TPZNetwork*)getOwnerRouter().getOwner())->routingRecord(source,destination,deltaX,deltaY,deltaZ);

   // A: Header manipulation.
   msg->setDelta(deltaX,0);
   msg->setDelta(deltaY,1);
   msg->setDelta(deltaZ,2);

   return true;
}

//*************************************************************************
//:
//  f: virtual Boolean stateChange();
//
//  d:
//:
//*************************************************************************

Boolean TPZSimpleRouterFlowBless :: stateChange()
{
   return true;
}


//*************************************************************************
//:
//  f: virtual Boolean outputWriting();
//
//  d:
//:
//*************************************************************************

Boolean TPZSimpleRouterFlowBless :: outputWriting()
{
   return true;
}

//*************************************************************************
//:
//  f: virtual Boolean dispatchEvent(const TPZEvent& event);
//
//  d:
//:
//*************************************************************************

Boolean TPZSimpleRouterFlowBless :: dispatchEvent(const TPZEvent& event)
{
   return true;
}

//*************************************************************************
//:
//  f: void cleanOutputInterfaces();
//
//  d:
//:
//*************************************************************************

void TPZSimpleRouterFlowBless :: cleanOutputInterfaces()
{
   if( cleanInterfaces() )
   {
      unsigned i,j;
      TPZSimpleRouter& simpleRouter = (TPZSimpleRouter&)getComponent();

      forInterfaz(i,simpleRouter.numberOfOutputs())
      {
         forInterfaz(j,outputInterfaz(i)->numberOfCV() )
            outputInterfaz(i)->clearData(j);
      }
   }
}

//*************************************************************************
//:
//  f: void cleanInputInterfaces();
//
//  d:
//:
//*************************************************************************

void TPZSimpleRouterFlowBless :: cleanInputInterfaces()
{
   if( cleanInterfaces() )
   {
      unsigned i,j;
      TPZSimpleRouter& simpleRouter = (TPZSimpleRouter&)getComponent();
      forInterfaz(i,simpleRouter.numberOfInputs())
      {
          // A: CV probably is 1 in BLESS since there is only 1 virtual channel at each port.
          forInterfaz(j,outputInterfaz(i)->numberOfCV() )
            inputInterfaz(i)->clearStop(j);
      }
   }
}

//*************************************************************************
//:
//  f: virtual Boolean controlAlgoritm(Boolean info=false, int delta=0);
//
//  d:
//:
//*************************************************************************

// A: I don't know what it is.
Boolean TPZSimpleRouterFlowBless :: controlAlgoritm(Boolean info, int delta)
{
   // Para evitar que sea una clase abstracta.
   return true;
}


//*************************************************************************
//:
//  f: virtual Boolean onReadyUp(unsigned interfaz, unsigned cv);
//
//  d:
//:
//*************************************************************************

Boolean TPZSimpleRouterFlowBless :: onReadyUp(unsigned interfaz, unsigned cv)
{
   TPZMessage* msg;
   inputInterfaz(interfaz)->getData(&msg);
   msg->incDistance(); // A: why?
   m_sync[interfaz]=msg;
 #ifndef NO_TRAZA
   TPZString texto4 = getComponent().asString() + " Message arriving = ";
   texto4 += msg->asString() ;
   TPZWRITE2LOG(texto4);
#endif
   return true;
}


//*************************************************************************
//:
//  f: virtual Boolean onStopUp(unsigned interfaz, unsigned cv);
//
//  d:
//:
//*************************************************************************

Boolean TPZSimpleRouterFlowBless :: onStopUp(unsigned interfaz, unsigned cv)
{
   return true;
}


//*************************************************************************
//:
//  f: virtual Boolean onStopDown(unsigned interfaz, unsigned cv);
//
//  d:
//:
//*************************************************************************

Boolean TPZSimpleRouterFlowBless :: onStopDown(unsigned interfaz, unsigned cv)
{
   return true;
}

//*************************************************************************
//:
//  f: TPZString getStatus() const;
//
//  d:
//:
//*************************************************************************

TPZString TPZSimpleRouterFlowBless :: getStatus() const
{
   TPZSimpleRouter& crb = (TPZSimpleRouter&)getComponent();
   TPZString rs = crb.asString() + TPZString(":\tIn stop= ");

   int i, channel;
   for( i=1; i<=crb.numberOfInputs(); i++ )
   {
      if( inputInterfaz(i)->isStopActive() )
         rs+= TPZString("I") + TPZString(i) + " ";
   }
   rs += TPZString(".\tOut stop= ");
   for( i=1; i<=crb.numberOfOutputs(); i++ )
   {
      if( crb.isLocalNodeOutput(i) )
      {
         if( outputInterfaz(i)->isStopActive() )
            rs+= TPZString("O") + TPZString(i) + " ";
      }

      else
      {
         channel=1;
         if( outputInterfaz(i)->isStopActive(channel) )
            rs+= TPZString("O") + TPZString(i) + TPZString(channel) + " ";
      }
   }

   return rs;
}

//*************************************************************************


// end of file
