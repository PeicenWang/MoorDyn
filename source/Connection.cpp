/*
 * Copyright (c) 2019 Matt Hall <mtjhall@alumni.uvic.ca>
 * 
 * This file is part of MoorDyn.  MoorDyn is free software: you can redistribute 
 * it and/or modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * MoorDyn is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MoorDyn.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Connection.h"
#include "Line.h"

// connection member functions

void Connection::setup(int number_in, int type_in, double r0_in[3], double M_in,
	double V_in, double F_in[3], double CdA_in, double Ca_in) 
{
	//props contains: Node      Type      X        Y         Z        M        V        FX       FY      FZ  CdA  Ca
	
	number = number_in;
	type = type_in;
	
	// store passed rod properties  >>>(and convert to numbers)<<<
	conX  = r0_in[0];
	conY  = r0_in[1];
	conZ  = r0_in[2];
	conM  = M_in ;
	conV  = V_in ;
	conFX = F_in[0];
	conFY = F_in[1];
	conFZ = F_in[2];
	conCdA= CdA_in;
	conCa = Ca_in;
	
	t=0.;		
	//beta = 0.0;
	
	nAttached = 0;  // start off with zero connections
		 
	// Start off at position specified in input file (will be overwritten for fairleads).
	// This is the starting point for connects and the permanent location of anchors.
	r[0] = conX; 
	r[1] = conY;  
	r[2] = conZ;  
	for (int I=0; I<3; I++) rd[I] = 0.0;
					
	//S.resize(3, vector< double >(3, 0.0));  // inverse mass matrices (3x3) for each node
	//M.resize(3, vector< double >(3, 0.0)); // node mass + added mass matrix
	//M_i.resize(3, vector< double >(3, 0.0));
};


// this function handles assigning a line to a connection node
void Connection::addLineToConnect(Line *theLine, int TopOfLine)
{
	if (wordy>0) cout << "L" << theLine->number << "->N" << number << " ";
	
	
	if (nAttached <10) // this is currently just a maximum imposed by a fixed array size.  could be improved.
	{
		Attached[nAttached] = theLine;
		Top[nAttached] = TopOfLine;
		nAttached += 1;
	}
};


// set fairlead ICs based on platform position IC
void Connection::initializeFairlead( double pX[], double TransMat[] )
{
	if (type==1)  // error check
	{	
		r[0] = TransMat[0]*conX + TransMat[1]*conY + TransMat[2]*conZ + pX[0];	// x
		r[1] = TransMat[3]*conX + TransMat[4]*conY + TransMat[5]*conZ + pX[1];	// y
		r[2] = TransMat[6]*conX + TransMat[7]*conY + TransMat[8]*conZ + pX[2];	// z
		
		for (int I=0; I<3; I++) rd[I] = 0.0;
		
		// also specify the above in the "vessel" arrays that prescribe the kinematics over the following time steps, for IC gen
		for (int J=0; J<3; J++)  {
			r_ves[J] = r[J];
			rd_ves[J] = 0.0;
		}
	}
	else  cout << "   Error: wrong connection type given to initializeFairlead().  Something's not right." << endl;
	// TODO: should handle.

	// pass kinematics to any attached lines
	for (int l=0; l < nAttached; l++) Attached[l]->setEndState(r, rd, Top[l]);
	
	return;
};

// set fairlead ICs based on fairlead-centric coupling (no platform stuff)
void Connection::initializeFairlead2( double pX[], double vX[] )
{
	if (type==1)  // error check
	{	
		for (int I=0; I<3; I++)
		{
			r[I] = pX[I];
			rd[I] = vX[I];
		}		
		// also specify the above in the "vessel" arrays that prescribe the kinematics over the following time steps, for IC gen
		for (int J=0; J<3; J++)  {
			r_ves[J] = r[J];
			rd_ves[J] = rd[J];
		}
	}
	else  cout << "   Error: wrong connection type given to initializeFairlead2().  Something's not right." << endl;
	// TODO: should handle.
	
	// pass kinematics to any attached lines
	for (int l=0; l < nAttached; l++) Attached[l]->setEndState(r, rd, Top[l]);

	return;
};



void Connection::initializeConnect( double* X )
{
	if (type==2)  // error check
	{	
		// assign initial node kinematics to state vector
		for (int I=0; I<3; I++)  {
			X[3 + I] = r[I];
			X[    I] = rd[I];
		}
	}
	else  cout << "   Error: wrong connection type given to initializeConnect().  Something's not right." << endl;
	// TODO: should handle.

	// pass kinematics to any attached lines so they have initial positions at this initialization stage
	for (int l=0; l < nAttached; l++) Attached[l]->setEndState(r, rd, Top[l]);
	
	return;
};


void Connection::initializeAnchor()
{
	if (type==0)  // error check
	{	
		// pass kinematics to any attached lines so they have initial positions at this initialization stage
		for (int l=0; l < nAttached; l++) Attached[l]->setEndState(r, rd, Top[l]);
		
	}
	else  cout << "   Error: wrong connection type given to initializeConnect().  Something's not right." << endl;
	// TODO: should handle.

	return;
};


// function to return connection position and velocity to Line object
void Connection::getConnectState(vector<double> &r_out, vector<double> &rd_out)
{
	for (int J=0; J<3; J++) {
		r_out[J] = r[J];
		rd_out[J] = rd[J];
	}
};



// function to return net force on connection (just to allow public reading of Fnet)
void Connection::getFnet(double Fnet_out[])
{
	for (int I=0; I<3; I++) 	Fnet_out[I] = Fnet[I];
};

// function to return mass matrix of connection
void Connection::getM(double M_out[3][3])
{
	for (int I=0; I<3; I++) 	
		for (int J=0; J<3; J++) 
			M_out[I][J] = M[I][J];
};


double Connection::GetConnectionOutput(OutChanProps outChan)
{
	if      (outChan.QType == PosX)  return  r[0];
	else if (outChan.QType == PosY)  return  r[1];
	else if (outChan.QType == PosZ)  return  r[2];
	else if (outChan.QType == VelX)  return  rd[0];
	else if (outChan.QType == VelY)  return  rd[1];
	else if (outChan.QType == VelZ)  return  rd[2];
	else if (outChan.QType == Ten )  return  sqrt(Fnet[0]*Fnet[0] + Fnet[1]*Fnet[1] + Fnet[2]*Fnet[2]);
	else if (outChan.QType == FX)    return  Fnet[0];  // added Oct 20
	else if (outChan.QType == FY)    return  Fnet[1];
	else if (outChan.QType == FZ)    return  Fnet[2];
	else
	{
		return 0.0;
		//ErrStat = ErrID_Warn
		//ErrMsg = ' Unsupported output quantity from Connect object requested.'
	}	
}


void Connection::setEnv(EnvCond env_in)
{
	env = env_in; // needed only for buoyancy calcs on connections that have a volumetric displacement
}


/*
// helper function to sum forces and mass from attached lines 
//  used for connect dynamics, fair/anch tensions, and now also rod rigidity constraint
void Connection::sumNetForceAndMass()
{
	// loop through each connected line, summing to get the final result
	
	int Nl = nAttached;				// number of attached line segments
	
	//cout << "Connection " << number << " nAttached is " << nAttached << endl;
	
	// clear before re-summing	
	for (int I=0; I<3; I++) { 
		Fnet[I] = 0;		
		for (int J=0; J<3; J++)
			M[I][J] = 0.0; 		
	}
	
	// loop through attached lines
	for (int l=0; l < Nl; l++)
	{
		// get quantities
		if (Top[l] == 0) 		// if attached to bottom/anchor of a line...
		{	(Attached[l])->getAnchStuff(Fnet_i, M_i);
			//cout << "Att. to bot of line " << (Attached[l])->number << " F:" << Fnet_i[0] << " " << Fnet_i[1] << " " << Fnet_i[2] << endl;
		}
		else 				// attached to top/fairlead
		{	(Attached[l])->getFairStuff(Fnet_i, M_i);
			//cout << "Att. to top of line " << (Attached[l])->number << " F:" << Fnet_i[0] << " " << Fnet_i[1] << " " << Fnet_i[2] << endl;
		}
			
		// sum quantitites
		for (int I=0; I<3; I++) {
			Fnet[I] += Fnet_i[I];
		
			for (int J=0; J<3; J++) 
				M[I][J] += M_i[I][J];					
		}
	}
		
	// add constant quantities for connection if applicable from input file
	Fnet[0] += conFX;
	Fnet[1] += conFY;
	Fnet[2] += conFZ + conV*env.rho_w*env.g - conM*env.g; 
	for (int I=0; I<3; I++) 	M[I][I] += conM;

}
*/


// function for boosting drag coefficients during IC generation	
void Connection::scaleDrag(double scaler)
{
	conCdA = conCdA*scaler;
	return;
}

// function to reset time after IC generation
void Connection::setTime(double time)
{
	t = time;
	return;
}

	
// pass the latest states to the connection ()
int Connection::setState( const double* X, const double time)
{
	t = time;
	
	// the kinematics should only be set with this function of it's an independent or body-attached connection
	if ((type==2) || (type==3)) // "connect" type
	{
		// from state values, get r and rdot values 
		for (int J=0; J<3; J++) 	
		{
			r[J]  = X[3 + J]; // get positions
			rd[J] = X[J]; // get velocities
		}		
		
		// pass kinematics to any attached lines (NEW)
		for (int l=0; l < nAttached; l++) Attached[l]->setEndState(r, rd, Top[l]);
				
		return 0;
	}
	else
	{	cout << "Error: setState called for wrong type of Connection" << endl;
		return -1; 
	}
}
	
	
// calculate the forces and state derivatives of the connectoin	
int Connection::getStateDeriv(double* Xd)
{	
	// the RHS is only relevant (there are only states to worry about) if it is a Connect type of Connection
	if (type==2) // "connect" type
	{
		if (t==0)   // with current IC gen approach, we skip the first call to the line objects, because they're set AFTER the call to the connects
		{		// above is no longer true!!! <<<
			for (int I=0; I<3; I++)  {
				Xd[3+I] = rd[I];  // velocities - these are unused in integration
				Xd[I] = 0.;     // accelerations - these are unused in integration
			}
		}
		else
		{					
			//cout << "ConRHS: m: " << M[0][0] << ", f: " << Fnet[0] << " " << Fnet[1] << " " << Fnet[2] << endl;
				
			doRHS();	
				
						
			// solve for accelerations in [M]{a}={f} using LU decomposition
			double M_tot[9];                     // serialize total mass matrix for easy processing
			for (int I=0; I<3; I++) for (int J=0; J<3; J++) M_tot[3*I+J]=M[I][J];
			double LU[9];                        // serialized matrix that will hold LU matrices combined
			Crout(3, M_tot, LU);                  // perform LU decomposition on mass matrix
			double acc[3];                        // acceleration vector to solve for
			solveCrout(3, LU, Fnet, acc);     // solve for acceleration vector
				
			// invert node mass matrix
			//inverse3by3(S, M);
			
			// RHS constant - (premultiplying force vector by inverse of mass matrix  ... i.e. rhs = S*Forces
			for (int I=0; I<3; I++) 
			{
				//double RHSI = 0.0; // temporary accumulator 
				//for (int J=0; J<3; J++) {
				//	RHSI += S[I][J] * Fnet[J]; //  matrix multiplication [S i]{Forces i}
				//}
				
				// update states
				Xd[3 + I] = rd[I];   // dxdt = V    (velocities)
				Xd[I] = acc[I];       // dVdt = RHS * A  (accelerations)
			}
		}
		return 0;
	}
	else
	{
		cout << "Error: wrong connection type sent to getStateDeriv().  " << endl;
		return -1;
	}
		
};


// calculate the force and mass contributions of the connect on the parent body (only for type 3 connects?)
int Connection::getNetForceAndMassContribution(double rBody[3], double Fnet_out[6], double M_out[6][6])
{
	doRHS();
		
	// make sure Fnet_out and M_out are zeroed first <<<<<<<<< can delete this <<<<<<<
	for (int J=0; J<6; J++)
	{	Fnet_out[J] = 0.0;
		for (int K=0; K<6; K++)
			M_out[J][K] = 0.0;
	}
	
			
	double rRel[3];    // position of connection relative to the body reference point (global orientation frame)
	for (int J=0; J<3; J++) rRel[J] = r[J] - rBody[J]; // vector from body reference point to node
			
	// convert segment net force into 6dof force about body ref point
	translateForce3to6DOF(rRel, Fnet, Fnet_out); 		
	
	// convert segment mass matrix to 6by6 mass matrix about body ref point
	translateMass3to6DOF(rRel, M, M_out);
	
	return 0;	
}

// this function calculates the forces and mass on the connection, including from attached lines
int Connection::doRHS()
{
	
	// loop through each connected line, summing to get the final result
	
	// clear before re-summing	
	for (int I=0; I<3; I++) { 
		Fnet[I] = 0;		
		for (int J=0; J<3; J++)
			M[I][J] = 0.0; 		
	}
	
	// loop through attached lines
	for (int l=0; l < nAttached; l++)
	{
		double Fnet_i[3] = {0.0};  double M_i[3][3] = {{0.0}};
		
		// get quantities
		if (Top[l] == 0) 		// if attached to bottom/anchor of a line...
		{	(Attached[l])->getAnchStuff(Fnet_i, M_i);
			//cout << "Att. to bot of line " << (Attached[l])->number << " F:" << Fnet_i[0] << " " << Fnet_i[1] << " " << Fnet_i[2] << endl;
		}
		else 				// attached to top/fairlead
		{	(Attached[l])->getFairStuff(Fnet_i, M_i);
			//cout << "Att. to top of line " << (Attached[l])->number << " F:" << Fnet_i[0] << " " << Fnet_i[1] << " " << Fnet_i[2] << endl;
		}
		
		// Process outline for line failure (yet to be coded):
		// 1. check if tension (of Fnet_i) exceeds line's breaking limit or if failure time has elapsed for line
		// 2. create new massless connect with same instantaneous kinematics as current connection
		// 3. disconnect line end from current connection and instead attach to new connect
		// The above may require rearrangement of connection indices, expansion of state vector, etc.
			
		// sum quantitites
		for (int I=0; I<3; I++) 
		{
			Fnet[I] += Fnet_i[I];
		
			for (int J=0; J<3; J++) 
				M[I][J] += M_i[I][J];					
		}
	}
		
	// add constant quantities for connection if applicable from input file
	Fnet[0] += conFX;
	Fnet[1] += conFY;
	Fnet[2] += conFZ + conV*env.rho_w*env.g - conM*env.g; 
	for (int I=0; I<3; I++) 	M[I][I] += conM;
			
	// add dynamic quantities for connection as specified in input file (feature added 2015/01/15)
	Fnet[0] -= 0.5*env.rho_w*rd[0]*abs(rd[0])*conCdA;
	Fnet[1] -= 0.5*env.rho_w*rd[1]*abs(rd[1])*conCdA;
	Fnet[2] -= 0.5*env.rho_w*rd[2]*abs(rd[2])*conCdA;
	for (int I=0; I<3; I++) 	M[I][I] += conV*env.rho_w*conCa;
		
	return 0;
}


// called at the beginning of each coupling step to update the boundary conditions (fairlead kinematics) for the proceeding line time steps
void Connection::initiateStep(double rFairIn[3], double rdFairIn[3], double time)
{	
	t0 = time; // set start time for BC functions
	
	if (type==1)  {  // if vessel type 
		// update values to fairlead position and velocity functions (fn of time)
		for (int J=0; J<3; J++)  {
			r_ves[J] = rFairIn[J];
			rd_ves[J] = rdFairIn[J];
		}
	}
	
	// do I want to get precalculated values here at each FAST time step or at each line time step?
};


void Connection::updateFairlead( const double time)
{	
	t = time;

	if (type==1) // vessel (moves with platform)		
	{						
		// set fairlead position and velocity based on BCs (linear model for now)
		for (int J=0; J<3; J++)  {
			r[J] = r_ves[J] + rd_ves[J]*(time-t0);
			rd[J] = rd_ves[J];
		}	
		
		// pass latest kinematics to any attached lines
		for (int l=0; l < nAttached; l++) Attached[l]->setEndState(r, rd, Top[l]);
	}
	else
		cout << "Error: wrong type sent to updateFairlead." << endl;
	
	return;
}
	
Connection::~Connection()
{
	// destory vectors
	//S.clear();  // inverse mass matrices (3x3) for each node
	//M.clear(); // node mass + added mass matrix
	//M_i.clear();
	
}

// new function to draw instantaneous line positions in openGL context
#ifdef USEGL
void Connection::drawGL(void)
{
	double radius = pow( conV/(4/3*pi), 0.33333);  //conV
	Sphere(r[0], r[1], r[2], radius);
};
#endif