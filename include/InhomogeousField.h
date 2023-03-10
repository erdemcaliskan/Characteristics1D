#ifndef INHOMOGENEOUS_FIELD__H
#define INHOMOGENEOUS_FIELD__H

#include "RandomVariable.h"

using namespace std;

/// val -> (a(val)^b + c)^d
class OneIHField_adjuster
{
public:
	OneIHField_adjuster();
	// returns true if abcd is read, false otherwise
	bool Read_OneIHField_adjuster(istream& in);
	// returns true if the value is changed
	bool Adjust_Value(double location_x, double& value);
	double a, b, c, d, e;
	double axm, axM;
	bool baxm, baxM;
};

// one inhomogeneous field for 1D domain
class OneIHField
{
public:
	OneIHField();
	~OneIHField();

	// inData: input for data
	// inConfig: read INPUTs below (all booleans given) plus bool num_Vals_and_x_Provided, bool containsRepeatingEndPeriodicVal
	// the last 3 values are provided as pointers to provide (isPeriodic, xM, xm) in case they are provided from outside
	// default_ValsAtVertices: for interfacial properties, naturally the random field values are read at vertices
	// for bulk properties such as E, rho it's the opposite and the values are more naturally read for segments
	// if OneIHField::valsAtVertices = 0, or 1 this entry is not used. However if valsAtVertices == -1, it's overwritten by default_ValsAtVertices
	void Read_Initialize_OneIHField(istream& inData, istream* inConfigPtr, int default_ValsAtVertices, bool* isPeriodicPtr = NULL, double* xMPtr = NULL, double* xmPtr = NULL, int resolutionFactor = 1, setStatOp_type sso = sso_mean_arithmetic);
	// resolutionFactor 
	//					== 0 or +/-1 nothing happens
	//					>  1  -> number of segments is DECREASED by this factor (e.g. if resolutionFactor ==  10 and numSegments = 1000 -> numSegments becoms 100)
	//					<  -1 -> number of segments is INCREASED by this factor (e.g. if resolutionFactor == -10 and numSegments = 1000 -> numSegments becoms 10000)
	void Modify_Resolution(int resolutionFactor, setStatOp_type sso);
	void Final_Adjustment_On_Values();

	// this gives vals[index]
	double getValueByIndex(unsigned int index) const;
	double getValueByCoord(double x) const;
	unsigned int getNumVertices() const { return numVertices; }
	unsigned int getNumSegments() const { return numSegments; }
	unsigned int getNumValues() const { return vals.size(); }
	void get_domain_range(double& xm_out, double& xM_out) const { xm_out = xm; xM_out = xM; }
	void get_xs(vector<double>& xsOut) const { xsOut = xs; }
	double get_xm() const { return xm; }
	double get_xM() const { return xM; }

	void Output(ostream& out);
	OneIHField(const OneIHField& other);
	OneIHField& operator=(const OneIHField& other);

private:
	// 0 -> values are for segments, 1 -> values for vertices, -1 -> values are given for vertices in the file (e.g. 1025 vertices) but the last one is ignored while reading and data is treated aas values for segments (e.g. only 1024 values are read)
	int valsAtVertices;
	// for uniform grid, only values are read from the file and xs are uniformly distributed from xm, xM
	// if the grid is not uniform, xs are also read from the file
	bool uniformGrid;
	bool isPeriodic;
	// domain minimum and maximum x
	double xm, xM;
	gRandVar* randVariableType;
	vector<OneIHField_adjuster> adjusters;
	unsigned int sz_adjusters;
	/////////////////////////////////////////////////////////////////////////////////
	// read from a file
	// values are given at vertices or are constant in elements connecting vertices based on valsAtVertices
	vector<double> vals;


	/////////////////////////////////////////////////////////////////////////////////
	// values finally set
	// vertex positions
	vector<double> xs;
	unsigned int numVertices;
	unsigned int numSegments;

private:
	bool num_Vals_and_x_Provided;
	bool containsRepeatingEndPeriodicVal;

	void Read_InstructionsOnly(istream* inConfigPtr, bool* isPeriodicPtr = NULL, double* xMPtr = NULL, double* xmPtr = NULL);
	void Read_DataOnly(istream& inData, int default_ValsAtVertices);

	// only reads vals and xs if uniformGrid == false
	// num_Vals_Provided: the size of number of values to be read given
	// containsRepeatingEndPeriodicVal: for periodic data if values are at vertices, the input file contains the repeating value at the end
	void Read_Vals_xs(istream& in, bool num_Vals_and_x_Provided, bool containsRepeatingEndPeriodicVal, int default_ValsAtVertices);
	void Finalize_spatialPositions();
	// is randomVariableType != NULL, the assumption is that read values are standard normal and they are mapped to target random type
	void Finalize_Values();
};

void TestInhomogeneousField(string baseNameWOExt = "TestFiles/file_v1_sz0_1");

#endif