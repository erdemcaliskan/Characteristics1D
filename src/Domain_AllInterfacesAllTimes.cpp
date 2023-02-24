#include "Domain_AllInterfacesAllTimes.h"
#include "SLDescriptorData.h"

//#define PRINT_DOMAIN_CONFIG VCPP
#define PRINT_DOMAIN_CONFIG 0

Domain_All_Interfaces_All_Times g_domain;

Domain_All_Interfaces_All_Times::Domain_All_Interfaces_All_Times()
{
	serialNumber = -1;
	x_min = 0;
	x_max = 0, L = 0;
	ring_R = 0.0;
	isPeriodic = false;
	num_subdomains = 0;
	num_bulks = 0;
	num_interfaces = 0;
	min_domain_del_t = DBL_MAX;
	max_domain_del_t = 0.0;

	b_x_min = false, b_subdomain_one_part_size = false, b_serialNumber = false, b_ring_R = false;
	interface_offset = 0;

	for (unsigned int i = 0; i < DiM; ++i)
	{
		directionalBCTypeLeftSide[i] = bct_Unspecified;
		directionalBCTypeRightSide[i] = bct_Unspecified;
	}
	hasSymOrAntiSymBC[SDL] = false;
	hasSymOrAntiSymBC[SDR] = false;

	do_space_spacetime_PP = true;
	pointSolution_maxIter_4PP = 10;
	pointSolution_relTol4Conv_4PP = 1e-4;
	numTimeStep_BulkInterfacePoints_Print_4PP = -20;
	numTimeStep_Interface_DSU_Fragment_Print_4PP = -200;
	numSpatialSubsegments_BulkInterfacePoints_Print_4PP = 1;
	useRepeatedSimpsonRuleForHigherOrders_4PP = false;

	io_type_InterfaceRawFinalSln_AllSpace_Print_4PP = iof_ascii;
	io_type_InterfaceRawScalars_AllSpace_Print_4PP = iof_ascii;
	numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP = 1;

	io_type_InterfaceRawFinalSln_AllTime_Print_4PP = iof_ascii;
	io_type_InterfaceRawScalars_AllTime_Print_4PP = iof_none;
	numSpaceStep_InterfaceRawFinalSlnScalars_AllTime_Print_4PP = -100;
	mainSubdomainNo = 0;
}

Domain_All_Interfaces_All_Times::~Domain_All_Interfaces_All_Times()
{
	{
		map<GID, SL_Bulk_Properties*>::iterator it, itb = bulk_elastic_map.begin(), ite = bulk_elastic_map.end();
		for (it = itb; it != ite; ++it)
			delete it->second;
	}
	{
		map<GID, SL_Interface_Fracture_PF*>::iterator it, itb = interface_fracture_map.begin(), ite = interface_fracture_map.end();
		for (it = itb; it != ite; ++it)
			delete it->second;
	}
	for (unsigned int i = 0; i < subdomains.size(); ++i)
		delete subdomains[i];
	for (unsigned int i = 0; i < bulks.size(); ++i)
	{
		if (bulks_deletable[i])
			delete bulks[i];
	}
	{
		map<pair<GID, GID>, SL_Elastic_InterfaceProperties*>::iterator it, itb = lr_IDS_2_ts_bulkProps.begin(), ite = lr_IDS_2_ts_bulkProps.end();
		for (it = itb; it != ite; ++it)
		{
			if (it->second != NULL)
				delete it->second;
		}
	}
	for (unsigned int i = 0; i < interfaces.size(); ++i)
		delete interfaces[i];
	Close_Files_RawData_OntTimeAllSpatialPoints();
}


void Domain_All_Interfaces_All_Times::Read_Initialize(string configNameIn, int serialNumberIn)
{
	configName = configNameIn;
	fstream in(configName.c_str(), ios::in);
	if (!in.is_open())
	{
		cout << "configName\n" << configName << '\n';
		THROW("Cannot open config name\n");
	}
	CopyFile2OutputDirectory(configName);
	Read_BaseData(in, serialNumberIn);
	PrepareForImpactIncidentEtcLoading();
	Form_subdomains();
	Form_Bulks_Interfaces_WithoutFormingConnections();
	Connect_Interfaces_Set_BC_Types__Form_PP();
	FinalizeImpactIncidentEtcLoading();
}

int Domain_All_Interfaces_All_Times::Main_Domain_Solution()
{
	Set_InitialCondition_step();
	if (!g_slf_conf->between_steps_adaptivity)
		return TimeStepsNonAdaptive();
	THROW("Nonadaptive option is not implemented\n");
}

void Domain_All_Interfaces_All_Times::Print(ostream& out) const
{
	out << "serialNumber\t" << serialNumber << '\n';
	out << "configName\t" << configName << '\n';
	out << "do_space_spacetime_PP\t" << do_space_spacetime_PP << '\n';
	out << "pointSolution_maxIter_4PP\t" << pointSolution_maxIter_4PP << '\n';
	out << "pointSolution_relTol4Conv_4PP\t" << pointSolution_relTol4Conv_4PP << '\n';
	out << "numTimeStep_BulkInterfacePoints_Print_4PP\t" << numTimeStep_BulkInterfacePoints_Print_4PP << '\n';
	out << "numTimeStep_Interface_DSU_Fragment_Print_4PP\t" << numTimeStep_Interface_DSU_Fragment_Print_4PP << '\n';
	out << "numSpatialSubsegments_BulkInterfacePoints_Print_4PP\t" << numSpatialSubsegments_BulkInterfacePoints_Print_4PP << '\n';
	out << "useRepeatedSimpsonRuleForHigherOrders_4PP\t" << useRepeatedSimpsonRuleForHigherOrders_4PP << '\n';

	out << "io_type_InterfaceRawFinalSln_AllSpace_Print_4PP\t" << io_type_InterfaceRawFinalSln_AllSpace_Print_4PP << '\n';
	out << "io_type_InterfaceRawScalars_AllSpace_Print_4PP\t" << io_type_InterfaceRawScalars_AllSpace_Print_4PP << '\n';
	out << "numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP\t" << numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP << '\n';

	out << "io_type_InterfaceRawFinalSln_AllTime_Print_4PP\t" << io_type_InterfaceRawFinalSln_AllTime_Print_4PP << '\n';
	out << "io_type_InterfaceRawScalars_AllTime_Print_4PP\t" << io_type_InterfaceRawScalars_AllTime_Print_4PP << '\n';
	out << "numSpaceStep_InterfaceRawFinalSlnScalars_AllTime_Print_4PP\t" << numSpaceStep_InterfaceRawFinalSlnScalars_AllTime_Print_4PP << '\n';


	out << "\n\nmin_domain_del_t\t" << min_domain_del_t << '\t';
	out << "max_domain_del_t\t" << max_domain_del_t << '\n';

	out << "bulk_elastic_map\t" << bulk_elastic_map.size() << '\n';
	for (map<GID, SL_Bulk_Properties*>::const_iterator it = bulk_elastic_map.begin(); it != bulk_elastic_map.end(); ++it)
	{
		out << "id" << it->first << '\n' << "bulkPtr\t" << it->second;
		if (it->second != NULL)
		{
			out << "\nbulk\n";
			it->second->Print(out, true);
		}
		out << '\n';
	}
	out << "interface_fracture_map\t" << interface_fracture_map.size() << '\n';
	for (map<GID, SL_Interface_Fracture_PF*>::const_iterator it = interface_fracture_map.begin(); it != interface_fracture_map.end(); ++it)
	{
		out << "id" << it->first << '\n' << "pfPtr\t" << it->second;
		if (it->second != NULL)
		{
			out << "\npf\n";
			it->second->PrintShort(out);
		}
		out << '\n';
	}

	out << "mainSubdomainNo\t" << mainSubdomainNo << '\n';
	out << "subdomain_config_names";	for (unsigned int i = 0; i < subdomain_config_names.size(); ++i) out << '\t' << subdomain_config_names[i]; out << '\n';
	out << "subdomain_one_part_size";	for (unsigned int i = 0; i < subdomain_one_part_size.size(); ++i) out << '\t' << subdomain_one_part_size[i]; out << '\n';
	out << "subdomain_bulk_start_nos";	for (unsigned int i = 0; i < subdomain_bulk_start_nos.size(); ++i) out << '\t' << subdomain_bulk_start_nos[i]; out << '\n';
	out << "subdomainNo4AllBulks";	for (unsigned int i = 0; i < subdomainNo4AllBulks.size(); ++i) out << '\t' << subdomainNo4AllBulks[i]; out << '\n';
	out << "bulk_interfaces_subdomains\n";
	for (unsigned int i = 0; i < bulk_interfaces_subdomains.size(); ++i)
	{
		out << "bulk_interfaces_subdomains" << i << '\n';
		bulk_interfaces_subdomains[i].PrintIndicesLengthsKeyRunParameters(out);
		out << '\n';
	}

	out << "isPeriodic\t" << isPeriodic << '\n';
	out << "interface_offset\t" << interface_offset << '\n';
	out << "b_x_min\t" << b_x_min << '\t';
	out << "b_subdomain_one_part_size\t" << b_subdomain_one_part_size << '\t';
	out << "b_serialNumber\t" << b_serialNumber << '\t';
	out << "b_serialNumberb_ring_R\t" << b_ring_R << '\n';

	out << "x_min\t" << x_min << '\t';
	out << "x_max\t" << x_max << '\t';
	out << "L\t" << L << '\t';
	out << "ring_R\t" << ring_R << '\n';
	out << "directionalBCTypeLeftSide";	for (unsigned int i = 0; i < DiM; ++i) out << '\t' << directionalBCTypeLeftSide[i]; out << '\n';
	out << "directionalBCTypeRightSide";	for (unsigned int i = 0; i < DiM; ++i) out << '\t' << directionalBCTypeRightSide[i]; out << '\n';

	out << "hasSymOrAntiSymBC";	for (unsigned int i = 0; i < NUM_SIDES; ++i) out << '\t' << hasSymOrAntiSymBC[i]; out << '\n';

	out << "\n\nSUBDOMAINS\n";
	out << "num_subdomains\t" << num_subdomains << '\n';
	for (unsigned int i = 0; i < subdomains.size(); ++i)
	{
		out << "subdomains" << i << "\tsubdomainsPtr\t" << subdomains[i] << '\n';
		if (subdomains[i] != NULL)
			subdomains[i]->Print(out);
	}

	out << "\n\nBULKS\n";
	out << "num_bulks\t" << num_bulks << '\n';
	for (unsigned int i = 0; i < bulks.size(); ++i)
	{
		out << "bulks" << i << "\tbulks\t" << bulks[i] << '\t' << "bulks_deletable\t" << bulks_deletable[i] << '\n';
		if (bulks[i] != NULL)
		{
			bulks[i]->Print(out, true);
			out << '\n';
		}
	}

	out << "\n\nBULK2BULK\n";
	out << "lr_IDS_2_ts_bulkProps\t" << lr_IDS_2_ts_bulkProps.size() << '\n';
	for (map<pair<GID, GID>, SL_Elastic_InterfaceProperties*>::const_iterator it = lr_IDS_2_ts_bulkProps.begin(); it != lr_IDS_2_ts_bulkProps.end(); ++it)
	{
		out << "( " << it->first.first << " , " << it->first.second << " )\n";
		out << it->second << '\n';
		if (it->second != NULL)
		{
			it->second->Print(out, true);
			out << '\n';
		}
	}

	out << "\n\nINTERFACES\n";
	out << "num_interfaces\t" << num_interfaces << '\n';
	out << "interface_xs\n";
	WriteVectorDouble(out, interface_xs);
	out << "\n";
	for (unsigned int i = 0; i < interfaces.size(); ++i)
	{
		out << "\nINTERFACE" << i << '\t';
		out << "x\t" << interface_xs[i] << '\t';
		out << "interface\t" << interfaces[i];
		if (interfaces[i] != NULL)
		{
			out << "\n";
			interfaces[i]->Print(out);
		}
		out << "\n";
	}
}

void Domain_All_Interfaces_All_Times::Read_BaseData(istream & in, int serialNumberIn)
{
	b_serialNumber = (serialNumberIn >= 0);
	if (b_serialNumber)
		serialNumber = serialNumberIn;

	string buf;
	READ_NSTRING(in, buf, buf);
	if (buf != "{")
	{
		if (buf == "}")
			return;
		else
		{
			while ((buf != "infile_domain") && (!in.eof()))
				READ_NSTRING(in, buf, buf);
			if (in.eof())
				THROW("Reached end of file looking for infile_domain\n");
			READ_NSTRING(in, buf, buf);
			if (buf != "{")
			{
				if (buf == "}")
					return;
				else
				{
					THROW("istream should start with {");
				}
			}
		}
	}
	READ_NSTRING(in, buf, buf);
	b_ring_R = false;
	while (buf != "}")
	{
		if (buf == "subdomain_config_names")
		{
			READ_NSTRING(in, buf, buf);
			if (buf != "{")
			{
				THROW("start of block should start with {");
			}
			READ_NSTRING(in, buf, buf);
			while (buf != "}")
			{
				subdomain_config_names.push_back(buf);
				READ_NSTRING(in, buf, buf);
			}
		}
		else if (buf == "x_min")
		{
			READ_NDOUBLE(in, buf, x_min);
			b_x_min = true;
		}
		else if (buf == "subdomain_one_part_size")
		{
			ReadVector(in, subdomain_one_part_size);
			b_subdomain_one_part_size = (subdomain_one_part_size.size() > 0);
		}
		else if (buf == "ring_R")
		{
			READ_NDOUBLE(in, buf, ring_R);
			b_ring_R = true;
		}
		else if (buf == "bulk_elastic_map")
		{
			READ_NSTRING(in, buf, buf);
			if (buf != "{")
			{
				THROW("start of block should start with {");
			}
			READ_NSTRING(in, buf, buf);
			while (buf != "}")
			{
				GID id;
				fromString(buf, id);
				map<GID, SL_Bulk_Properties*>::iterator it = bulk_elastic_map.find(id);
				if (it != bulk_elastic_map.end())
					delete it->second;
				bulk_elastic_map[id] = new SL_Bulk_Properties();
				it = bulk_elastic_map.find(id);
				it->second->Read_SL_Bulk_Properties(in, id);

				READ_NSTRING(in, buf, buf);
			}
		}
		else if (buf == "interface_fracture_map")
		{
			READ_NSTRING(in, buf, buf);
			if (buf != "{")
			{
				THROW("start of block should start with {");
			}
			READ_NSTRING(in, buf, buf);
			while (buf != "}")
			{
				GID id;
				fromString(buf, id);
				map<GID, SL_Interface_Fracture_PF*>::iterator it = interface_fracture_map.find(id);
				if (it != interface_fracture_map.end())
					delete it->second;
				interface_fracture_map[id] = new SL_Interface_Fracture_PF();
				it = interface_fracture_map.find(id);
				it->second->Read_SL_Interface_Fracture_PF(in, id);

				READ_NSTRING(in, buf, buf);
			}
		}
		else if (buf == "mainSubdomainNo")
		{
			READ_NINTEGER(in, buf, mainSubdomainNo);
		}
		else if (buf == "isPeriodic")
		{
			READ_NBOOL(in, buf, isPeriodic);
		}
		else if (buf == "directionalBCTypeLeftSide")
		{
			for (int i = 0; i < DiM; ++i)
				in >> directionalBCTypeLeftSide[i];
		}
		else if (buf == "directionalBCTypeRightSide")
		{
			for (int i = 0; i < DiM; ++i)
				in >> directionalBCTypeRightSide[i];
		}
		else if (buf == "do_space_spacetime_PP")
		{
			READ_NBOOL(in, buf, do_space_spacetime_PP);
		}
		else if (buf == "pointSolution_maxIter_4PP")
		{
			READ_NINTEGER(in, buf, pointSolution_maxIter_4PP);
		}
		else if (buf == "pointSolution_relTol4Conv_4PP")
		{
			READ_NDOUBLE(in, buf, pointSolution_relTol4Conv_4PP);
		}
		else if (buf == "numTimeStep_BulkInterfacePoints_Print_4PP")
		{
			READ_NINTEGER(in, buf, numTimeStep_BulkInterfacePoints_Print_4PP);
		}
		else if (buf == "numTimeStep_Interface_DSU_Fragment_Print_4PP")
		{
			READ_NINTEGER(in, buf, numTimeStep_Interface_DSU_Fragment_Print_4PP);
		}
		else if (buf == "numSpatialSubsegments_BulkInterfacePoints_Print_4PP")
		{
			READ_NINTEGER(in, buf, numSpatialSubsegments_BulkInterfacePoints_Print_4PP);
		}
		else if (buf == "useRepeatedSimpsonRuleForHigherOrders_4PP")
		{
			READ_NBOOL(in, buf, useRepeatedSimpsonRuleForHigherOrders_4PP);
		}
		else if (buf == "io_type_InterfaceRawFinalSln_AllSpace_Print_4PP")
		{
			in >> io_type_InterfaceRawFinalSln_AllSpace_Print_4PP;
		}
		else if (buf == "io_type_InterfaceRawScalars_AllSpace_Print_4PP")
		{
			in >> io_type_InterfaceRawScalars_AllSpace_Print_4PP;
		}
		else if (buf == "numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP")
		{
			READ_NINTEGER(in, buf, numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP);
		}
		else if (buf == "io_type_InterfaceRawFinalSln_AllTime_Print_4PP")
		{
			in >> io_type_InterfaceRawFinalSln_AllTime_Print_4PP;
		}
		else if (buf == "io_type_InterfaceRawScalars_AllTime_Print_4PP")
		{
			in >> io_type_InterfaceRawScalars_AllTime_Print_4PP;
		}
		else if (buf == "numSpaceStep_InterfaceRawFinalSlnScalars_AllTime_Print_4PP")
		{
			READ_NINTEGER(in, buf, numSpaceStep_InterfaceRawFinalSlnScalars_AllTime_Print_4PP);
		}
		else
		{
			cout << "buf:\t" << buf << '\n';
			THROW("invalid option\n");
		}
		READ_NSTRING(in, buf, buf);
	}
	if (b_ring_R && !b_x_min)
	{
		b_x_min = true;
		x_min = 0.0;
	}
	if (g_low_disk_space)
	{
		numSpaceStep_InterfaceRawFinalSlnScalars_AllTime_Print_4PP = 0;
		numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP = 0;

		numTimeStep_BulkInterfacePoints_Print_4PP = -1;
		if (numTimeStep_Interface_DSU_Fragment_Print_4PP < 0)
			numTimeStep_Interface_DSU_Fragment_Print_4PP = MAX(numTimeStep_Interface_DSU_Fragment_Print_4PP , -100);
	}
	if (numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP == 0)
	{
		io_type_InterfaceRawFinalSln_AllSpace_Print_4PP = iof_none;
		io_type_InterfaceRawScalars_AllSpace_Print_4PP = iof_none;
	}
	if ((io_type_InterfaceRawFinalSln_AllSpace_Print_4PP == iof_none) && (io_type_InterfaceRawScalars_AllSpace_Print_4PP == iof_none))
		numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP = 0;
	if (numSpaceStep_InterfaceRawFinalSlnScalars_AllTime_Print_4PP == 0)
	{
		io_type_InterfaceRawFinalSln_AllTime_Print_4PP = iof_none;
		io_type_InterfaceRawScalars_AllTime_Print_4PP = iof_none;
	}
	if ((io_type_InterfaceRawFinalSln_AllTime_Print_4PP == iof_none) && (io_type_InterfaceRawScalars_AllTime_Print_4PP == iof_none))
		numSpaceStep_InterfaceRawFinalSlnScalars_AllTime_Print_4PP = 0;
}

void Domain_All_Interfaces_All_Times::PrepareForImpactIncidentEtcLoading()
{
	if (g_SL_desc_data.tdLoadComputer == NULL)
		return;
	bool isNeumannOrDirichlet = ((g_SL_desc_data.tdLoadType == lmt_Neumann) || (g_SL_desc_data.tdLoadType == lmt_Dirichlet));
	bool isLeft = (g_SL_desc_data.tdLoadSide == SDL);
	bool need_left_out_interface = (!isNeumannOrDirichlet || !isLeft);
	bool need_right_out_interface = (!isNeumannOrDirichlet || isLeft);
	GID inteface_id;
	map<GID, SL_Interface_Fracture_PF*>::iterator fit, fitli, fitri;
	GID bulk_id;
	map<GID, SL_Bulk_Properties*>::iterator bit, bit2;
	if (need_left_out_interface) // make sure the interface for this exists
	{
		inteface_id = g_interfaceFlags_IncImp[LO_INT];
		fit = interface_fracture_map.find(inteface_id);
		if (fit == interface_fracture_map.end())
		{
			interface_fracture_map[inteface_id] = new SL_Interface_Fracture_PF();
			fit = interface_fracture_map.find(inteface_id);
		}
		fit->second->damageOffOnMix = sl_interfacial_damage_off;

		bulk_id = g_bulkFlags_IncImp[SDL];
		bit = bulk_elastic_map.find(bulk_id);
		if (bit == bulk_elastic_map.end())
		{
			bulk_elastic_map[bulk_id] = new SL_Bulk_Properties();
			bit = bulk_elastic_map.find(bulk_id);
			bit->second->Initialize_FromInputParas();
		}
	}

	if (need_right_out_interface)
	{
		inteface_id = g_interfaceFlags_IncImp[RO_INT];
		fit = interface_fracture_map.find(inteface_id);
		if (fit == interface_fracture_map.end())
		{
			interface_fracture_map[inteface_id] = new SL_Interface_Fracture_PF();
			fit = interface_fracture_map.find(inteface_id);
		}
		fit->second->damageOffOnMix = sl_interfacial_damage_off;

		bulk_id = g_bulkFlags_IncImp[SDR];
		bit = bulk_elastic_map.find(bulk_id);
		if (bit == bulk_elastic_map.end())
		{
			bulk_elastic_map[bulk_id] = new SL_Bulk_Properties();
			bit = bulk_elastic_map.find(bulk_id);

			bit2 = bulk_elastic_map.find(g_bulkFlags_IncImp[SDL]);
			if (bit2 != bulk_elastic_map.end())
				*(bit->second) = *(bit2->second);
			bit->second->Initialize_FromInputParas();
		}
	}
	// now ensuring interfaces inside the domain exist
	inteface_id = g_interfaceFlags_IncImp[LI_INT];
	fitli = interface_fracture_map.find(inteface_id);
	if (fitli == interface_fracture_map.end())
	{
		interface_fracture_map[inteface_id] = new SL_Interface_Fracture_PF();
		fitli = interface_fracture_map.find(inteface_id);
	}
	fitli->second->damageOffOnMix = sl_interfacial_damage_off;

	inteface_id = g_interfaceFlags_IncImp[RI_INT];
	fitri = interface_fracture_map.find(inteface_id);
	if (fitri == interface_fracture_map.end())
	{
		interface_fracture_map[inteface_id] = new SL_Interface_Fracture_PF();
		fitri = interface_fracture_map.find(inteface_id);
	}
	fitri->second->damageOffOnMix = sl_interfacial_damage_off;
	/// now setting the interfaces on a case basis
	mainSubdomainNo = 1;
	if (isNeumannOrDirichlet)
	{
		BoundaryConditionT bc = bct_Neumann;
		if (g_SL_desc_data.tdLoadType == lmt_Dirichlet)
			bc = bct_Dirichlet;
		if (isLeft)
		{
			for (unsigned int d = 0; d < DiM; ++d)
				directionalBCTypeLeftSide[d] = bc;
			for (unsigned int d = 0; d < DiM; ++d)
				if (directionalBCTypeRightSide[d] == bct_Undecided)
					directionalBCTypeRightSide[d] = bct_Characteristics;
			mainSubdomainNo = 0;
		}
		else
		{
			for (unsigned int d = 0; d < DiM; ++d)
				directionalBCTypeRightSide[d] = bc;
			for (unsigned int d = 0; d < DiM; ++d)
				if (directionalBCTypeLeftSide[d] == bct_Undecided)
					directionalBCTypeLeftSide[d] = bct_Characteristics;
		}
	}
	else if (g_SL_desc_data.tdLoadType == lmt_Incident)
	{
		// all inter-subdomain interfaces are bonded which is fine. Just take care of BCs
		for (unsigned int d = 0; d < DiM; ++d)
			if (directionalBCTypeLeftSide[d] == bct_Undecided)
				directionalBCTypeRightSide[d] = bct_Characteristics;
		for (unsigned int d = 0; d < DiM; ++d)
			if (directionalBCTypeLeftSide[d] == bct_Undecided)
				directionalBCTypeLeftSide[d] = bct_Characteristics;
	}
	else if (g_SL_desc_data.tdLoadType == lmt_Impact)
	{
		// first take care of fully debonded interface between the projectile and body
		SL_Interface_Fracture_PF* interfacePF;
		if (isLeft)
			interfacePF = fitli->second;
		else
			interfacePF = fitri->second;
		interfacePF->damageOffOnMix = sl_interfacial_damage_on;
		interfacePF->contactOffOnMix = sl_contact_mixed;
		interfacePF->slipOffOnMix = sl_slip_off;

		// now taking care of BCs
		BoundaryConditionT bc = bct_Neumann;
		if (isLeft)
		{
			for (unsigned int d = 0; d < DiM; ++d)
				directionalBCTypeLeftSide[d] = bc;
			for (unsigned int d = 0; d < DiM; ++d)
				if (directionalBCTypeRightSide[d] == bct_Undecided)
					directionalBCTypeRightSide[d] = bct_Characteristics;
		}
		else
		{
			for (unsigned int d = 0; d < DiM; ++d)
				directionalBCTypeRightSide[d] = bc;
			for (unsigned int d = 0; d < DiM; ++d)
				if (directionalBCTypeLeftSide[d] == bct_Undecided)
					directionalBCTypeLeftSide[d] = bct_Characteristics;
		}
	}
}

void Domain_All_Interfaces_All_Times::FinalizeImpactIncidentEtcLoading()
{
	if (g_SL_desc_data.tdLoadComputer == NULL)
		return;
	bool isNeumannOrDirichlet = ((g_SL_desc_data.tdLoadType == lmt_Neumann) || (g_SL_desc_data.tdLoadType == lmt_Dirichlet));
	bool isLeft = (g_SL_desc_data.tdLoadSide == SDL);
	double amientProjLength = 0;
	double Ein, rhoin, Eout = -1, rhoout = -1;
	int interfaceNo = -1;
	if (isNeumannOrDirichlet)
	{
		if (isLeft)
			interfaceNo = subdomain_bulk_start_nos[0];
		else
			interfaceNo = subdomain_bulk_start_nos[1];
	}
	else
	{
		if (isLeft)
		{
			amientProjLength = subdomains[0]->getTotalLength();
			interfaceNo = subdomain_bulk_start_nos[1];
		}
		else
		{
			amientProjLength = subdomains[2]->getTotalLength();
			interfaceNo = subdomain_bulk_start_nos[2];
		}
	}
	SL_OneInterfaceAllTimes *interfacePtr = interfaces[interfaceNo];
	if (isLeft)
	{
		Ein = interfacePtr->ts_bulkProps->bulk_rightPtr->E_iso;
		rhoin = interfacePtr->ts_bulkProps->bulk_rightPtr->rho;
		if (!isNeumannOrDirichlet)
		{
			Eout = interfacePtr->ts_bulkProps->bulk_leftPtr->E_iso;
			rhoout = interfacePtr->ts_bulkProps->bulk_leftPtr->rho;
		}
	}
	else
	{
		Ein = interfacePtr->ts_bulkProps->bulk_leftPtr->E_iso;
		rhoin = interfacePtr->ts_bulkProps->bulk_leftPtr->rho;
		if (!isNeumannOrDirichlet)
		{
			Eout = interfacePtr->ts_bulkProps->bulk_rightPtr->E_iso;
			rhoout = interfacePtr->ts_bulkProps->bulk_rightPtr->rho;
		}
	}
	double delT = g_slf_conf->uniform_del_t;
		if (delT < 0)
			delT *= -min_domain_del_t;
	g_SL_desc_data.Finalize_tdLoadParameters(g_slf_conf->terminate_run_target_time, delT, amientProjLength, Ein, rhoin, Eout, rhoout);
}

void Domain_All_Interfaces_All_Times::Form_subdomains()
{
	string fileName;
	string specificName = "mainSubdomainNo";
	bool addUnderline = true;
	GetSubdomainIndexed_TimeIndexed_FileName(fileName, -1, -1, specificName, "txt", addUnderline);
	fstream out(fileName.c_str(), ios::out);
	out << mainSubdomainNo << '\n';
	out.close();

	// 0 is default fracture interface flag: generally fully bonded. If the user does not provide it, it's created here
	map<GID, SL_Interface_Fracture_PF*>::iterator it = interface_fracture_map.find(0);
	if (it == interface_fracture_map.end())
	{
		interface_fracture_map[0] = new SL_Interface_Fracture_PF();
		interface_fracture_map[0]->CreateBonded_PF();
	}

	int *serialNumberPtr = NULL;
	if (b_serialNumber)
		serialNumberPtr = &serialNumber;
#if RING_PROBLEM
	isPeriodic = true;
#endif
	unsigned int num_subdomains_tmp = subdomain_config_names.size();
	interface_offset = 1;
	if (isPeriodic)
		interface_offset = 0;

	if (b_x_min && b_subdomain_one_part_size)
	{
		if (subdomain_one_part_size.size() != num_subdomains_tmp)
		{
			THROW("(subdomain_one_part_size.size() != num_subdomains_tmp)\n");
		}
		double xm = x_min, xM;
		unsigned int si = 0;
		for (unsigned int sii = 0; sii < num_subdomains_tmp; ++sii)
		{
			xM = xm + subdomain_one_part_size[sii];
			Subdomain_ElasticFractureModifier* subdomain = new Subdomain_ElasticFractureModifier();
			string configNameIn4Subdomain = subdomain_config_names[sii];
			if (configNameIn4Subdomain == "infile")
				configNameIn4Subdomain = configName;

			subdomain->Read_Subdomain_ElasticFractureModifier(sii, configNameIn4Subdomain, serialNumberPtr, &isPeriodic, &xM, &xm);
			bool isActive = Is_ActiveSubdomain(sii, subdomain, si, num_subdomains_tmp);
			if (isActive)
			{
				double totalLength = subdomain_one_part_size[sii] * subdomain->numRepeatSequence;
				xm += totalLength;
				subdomains.push_back(subdomain);
			}
			else
				delete subdomain;
		}
	}
	else
	{
		unsigned int si = 0;
		for (unsigned int sii = 0; sii < num_subdomains_tmp; ++sii)
		{
			Subdomain_ElasticFractureModifier* subdomain = new Subdomain_ElasticFractureModifier();
			string configNameIn4Subdomain = subdomain_config_names[sii];
			if (configNameIn4Subdomain == "infile")
				configNameIn4Subdomain = configName;

			if ((si == 0) && (b_x_min))
				subdomain->Read_Subdomain_ElasticFractureModifier(sii, configNameIn4Subdomain, serialNumberPtr, &isPeriodic, NULL, &x_min);
			else
				subdomain->Read_Subdomain_ElasticFractureModifier(sii, configNameIn4Subdomain, serialNumberPtr, &isPeriodic);
			bool isActive = Is_ActiveSubdomain(sii, subdomain, si, num_subdomains_tmp);
			if (isActive)
			{
				subdomains.push_back(subdomain);
				++si;
			}
			else
				delete subdomain;
		}
		x_min = subdomains[0]->interface_xs[0];
	}
	num_subdomains = subdomains.size();

	if ((io_type_InterfaceRawFinalSln_AllSpace_Print_4PP != iof_none) || (io_type_InterfaceRawScalars_AllSpace_Print_4PP != iof_none))
	{
		outScalars_fixed_time.resize(num_subdomains);
		outFinalSln_fixed_time.resize(num_subdomains);
		for (unsigned int si = 0; si < num_subdomains; ++si)
		{
			outScalars_fixed_time[si] = NULL;
			outFinalSln_fixed_time[si] = NULL;
		}
	}
}

bool Domain_All_Interfaces_All_Times::Is_ActiveSubdomain(int sii, Subdomain_ElasticFractureModifier*& subdomain, unsigned int& si, unsigned int num_subdomains_tmp)
{
	if (g_SL_desc_data.tdLoadComputer != NULL)
	{
		if ((g_SL_desc_data.tdLoadType == lmt_Neumann) || (g_SL_desc_data.tdLoadType == lmt_Dirichlet))
		{
			// removing ambient from the load application side
			if (g_SL_desc_data.tdLoadSide == SDL)
			{
				if (sii == 0)
					return false;
			}
			else if (g_SL_desc_data.tdLoadSide == SDR)
			{
				if (sii == num_subdomains_tmp - 1)
					return false;
			}
		}
		if (sii == 0)
		{
			for (unsigned int bi = 0; bi < subdomain->bulkMs.size(); ++bi)
			{
				Bulk_Elastic_Modifier* bemPtr = &subdomain->bulkMs[bi];
				bemPtr->bulk_flag = g_bulkFlags_IncImp[SDL];
			}
//			subdomain->intefaceMs[subdomain->intefaceMs.size() - 1].interface_flag = g_interfaceFlags_IncImp[LI_INT];
		}
//		else if (sii == num_subdomains_tmp - 2)
//		{
//			subdomain->intefaceMs[subdomain->intefaceMs.size() - 1].interface_flag = g_interfaceFlags_IncImp[RI_INT];
//		}
		else if (sii == num_subdomains_tmp - 1)
		{
			for (unsigned int bi = 0; bi < subdomain->bulkMs.size(); ++bi)
			{
				Bulk_Elastic_Modifier* bemPtr = &subdomain->bulkMs[bi];
				bemPtr->bulk_flag = g_bulkFlags_IncImp[SDR];
			}
//			subdomain->intefaceMs[subdomain->intefaceMs.size() - 1].interface_flag = g_interfaceFlags_IncImp[RO_INT];
		}
	}
	return true;
}

void Domain_All_Interfaces_All_Times::Form_Bulks_Interfaces_WithoutFormingConnections()
{
	subdomain_bulk_start_nos.resize(num_subdomains + 1);
	num_bulks = 0;
	subdomain_bulk_start_nos[0] = 0;
	for (unsigned int si = 0; si < num_subdomains; ++si)
	{
		num_bulks += subdomains[si]->getTotalNumBulk();
		subdomain_bulk_start_nos[si + 1] = num_bulks;
	}
	num_interfaces = num_bulks + interface_offset;

	bulks.resize(num_bulks);
	bulks_deletable.resize(num_bulks);

	interfaces.resize(num_interfaces);
	interface_xs.resize(num_interfaces);

	double x = x_min;

	SL_Bulk_Properties *bulkPropPtr, *new_bulkPropPtr;
	Bulk_Elastic_Modifier* bulkModifierPtr;
	SL_Interface_Fracture_PF* interfacePF;
	Interface_Fracture_Modifier *interfaceModifierPtr;
	SL_OneInterfaceAllTimes* interfacePtr;

	interface_xs[0] = x_min;

	unsigned int bulk_cntr = 0, interface_cntr;
	for (unsigned int si = 0; si < num_subdomains; ++si)
	{
		Subdomain_ElasticFractureModifier *subdomain = subdomains[si];
		for (unsigned int ri = 0; ri < subdomain->numRepeatSequence; ++ri)
		{
			for (unsigned int bi = 0; bi < subdomain->num_bulk_one_seq; ++bi)
			{
				bulkModifierPtr = &subdomain->bulkMs[bi];

				// finding the corresponding bulk pointer
				map<GID, SL_Bulk_Properties*>::iterator it = bulk_elastic_map.find(bulkModifierPtr->bulk_flag);
				// if it cannot find this material a default iso with E = 1, rho = 1 is formed
				if (it == bulk_elastic_map.end())
				{
					bulkPropPtr = new SL_Bulk_Properties();
					bulkPropPtr->Form_Iso_E1Rho1Default();
					bulk_elastic_map[bulkModifierPtr->bulk_flag] = bulkPropPtr;
				}
				else
					bulkPropPtr = it->second;

				// see if the modifier is changing the bulk found or not
				if (!bulkModifierPtr->b_modifies)
				{
					new_bulkPropPtr = bulkPropPtr;
					bulks_deletable[bulk_cntr] = false;
				}
				else
				{
					new_bulkPropPtr = new SL_Bulk_Properties();
					new_bulkPropPtr->Initialize_FromOther_withFactors(*bulkPropPtr, bulkModifierPtr->CFactor, bulkModifierPtr->rhoFactor, bulkModifierPtr->dampingFactor);
					bulks_deletable[bulk_cntr] = true;
				}
				bulks[bulk_cntr] = new_bulkPropPtr;

				////// now forming the interface fracture part and position for this
				interface_cntr = bulk_cntr + interface_offset;
				// x of the interface to the right of this bulk
				x += bulkModifierPtr->length;
				interface_xs[interface_cntr] = x;

				// looking for the modifier
				interfaceModifierPtr = &subdomain->intefaceMs[bi];
				// getting interface fracture ...
				map<GID, SL_Interface_Fracture_PF*>::iterator itf = interface_fracture_map.find(interfaceModifierPtr->interface_flag);
				if (itf == interface_fracture_map.end())
				{
					cout << "interface flag: interfaceModifierPtr->interface_flag\t" << interfaceModifierPtr->interface_flag << '\n';
					THROW("Fracture interface property is not given\n");
				}
				interfacePF = itf->second;

				interfacePtr = new SL_OneInterfaceAllTimes();
				interfacePtr->interface_x = x;
				interfacePtr->sigmaCFactor = interfaceModifierPtr->sigmaFactor;
				interfacePtr->deltaCFactor = interfaceModifierPtr->deltaFactor;
				interfacePtr->iniDamage = interfaceModifierPtr->iniDamage;
				interfacePtr->Set_EF_Properties(interfacePF);
				interfacePtr->interface_flag = interfaceModifierPtr->interface_flag;
				interfacePtr->interface_pos = interface_cntr;

				interfaces[interface_cntr] = interfacePtr;

				// incrementing the bulk counter
				++bulk_cntr;
			}
		}
	}
	x_max = interface_xs[num_interfaces - 1];
	L = x_max - x_min;
#if RING_PROBLEM
	if (!b_ring_R)
		ring_R = 0.5 * L / PI;
	else
	{
		double newLength = 2.0 * PI * ring_R;
		double lengthFactor = newLength / L;
		if (fabs(lengthFactor - 1.0) > 1e-7)
		{
			// scaling all lengths
			x_min *= lengthFactor;
			x_max *= lengthFactor;
			L = newLength;
			for (unsigned j = 0; j < num_interfaces; ++j)
			{
				interfaces[j]->interface_x *= lengthFactor;
				interface_xs[j] *= lengthFactor;
			}
		}
	}
#endif
}

void Domain_All_Interfaces_All_Times::Connect_Interfaces_Set_BC_Types__Form_PP()
{
	bool isNeumannOrDirichlet = ((g_SL_desc_data.tdLoadType == lmt_Neumann) || (g_SL_desc_data.tdLoadType == lmt_Dirichlet));

	Generate_subdomain_nos_for_all_bulks(subdomainNo4AllBulks);
	bulk_interfaces_subdomains.resize(num_subdomains);
	postProcessing_subdomains.resize(num_subdomains);
	unsigned int st, en, sz, pos;
	for (unsigned int si = 0; si < num_subdomains; ++si)
	{
		st = subdomain_bulk_start_nos[si];
		en = subdomain_bulk_start_nos[si + 1];
		sz = en - st;
		OneSubdomain_All_bulksConnectivityInfo* osabci = &bulk_interfaces_subdomains[si];
		osabci->bulk_cn_st = st;
		osabci->bulk_cn_en = en;
		osabci->numSegments = sz;
		osabci->subdomain_bulk_segments.resize(sz);
		for (unsigned int j = 0; j < sz; ++j)
		{
			pos = j + st;
			osabci->subdomain_bulk_segments[j].bulkPtr = bulks[pos];
			osabci->subdomain_bulk_segments[j].bulk_cntr = pos;
		}
		osabci->subdomain_interfaces.resize(sz + 1);
		osabci->subdomain_interface_xs.resize(sz + 1);
		osabci->subdomain_number = si;
	}
	unsigned int sd_numL, sd_numR, bulk_posL, bulk_posR;
	OneSubdomain_All_bulksConnectivityInfo* osabci;
	OneBulktwoSideInterfaceInfo* obsifi;

	min_domain_del_t = DBL_MAX;
	max_domain_del_t = 0.0;

	// first taking care of boundary interfaces for non-periodic case
	SL_OneInterfaceAllTimes* interfacePtr;
	double* periodic_totalLength = &L;
	if (!isPeriodic)
	{
		if (interfaces[0] == NULL)
			interfaces[0] = new SL_OneInterfaceAllTimes();

		periodic_totalLength = NULL;
		// there are left and right side interfaxces
		// left interface
		unsigned int indices[2], index;
		indices[0] = 0;
		indices[1] = num_interfaces - 1;
		for (unsigned int i = 0; i < 2; ++i)
		{
			index = indices[i];
			interfacePtr = interfaces[index];
			interfacePtr->deltaCFactor = 1.0;
			interfacePtr->sigmaCFactor = 1.0;
			interfacePtr->interface_flag = 0;
			interfacePtr->interface_pos = index;
			// now setting the neighbor sequence, x, delx, delt ...
			SL_Elastic_InterfaceProperties* ts_bulkPropsInOut = NULL;
			bool hasSymAntiSym = false;
			if (i == 0)
			{
				SL_Bulk_Properties* bLeft = NULL;
				InterfaceLocation1DT interfaceLoc = ilt_left;
				for (unsigned int d = 0; d < DiM; ++d)
					if ((directionalBCTypeLeftSide[d] == bct_Symmetric) || (directionalBCTypeLeftSide[d] == bct_AntiSymmetric))
					{
						hasSymAntiSym = true;
						bLeft = bulks[0];
						interfaceLoc = ilt_twoSided;
					}
				hasSymOrAntiSymBC[SDL] = hasSymAntiSym;

				interfacePtr->interface_x = x_min;
				interfacePtr->Set_left_right_TimeSequenceData(interfaceLoc, directionalBCTypeLeftSide,
					bLeft, bulks[0],
					true, ts_bulkPropsInOut,
					NULL, interfaces[1], NULL);
				interfacePtr->sides_bulk_index[SDL] = -1;
				interfacePtr->sides_bulk_index[SDR] = 0;

				sd_numL = 0;
				bulk_posL = 0;
				osabci = &bulk_interfaces_subdomains[sd_numL];
				obsifi = &osabci->subdomain_bulk_segments[bulk_posL];
				//		obsifi->bulk_cntr = 0;
				//		obsifi->bulkPtr = bulks[0];
				obsifi->interfaceLeftOfBulk_cntr = index;
				obsifi->interfaceLeftOfBulkPtr = interfacePtr;
				osabci->subdomain_interfaces[bulk_posL] = interfacePtr;
			}
			else
			{
				InterfaceLocation1DT interfaceLoc = ilt_right;
				SL_Bulk_Properties* bRight = NULL;
				for (unsigned int d = 0; d < DiM; ++d)
					if ((directionalBCTypeRightSide[d] == bct_Symmetric) || (directionalBCTypeRightSide[d] == bct_AntiSymmetric))
					{
						hasSymAntiSym = true;
						bRight = bulks[num_bulks - 1];
						interfaceLoc = ilt_twoSided;
					}
				hasSymOrAntiSymBC[SDR] = hasSymAntiSym;

				interfacePtr->interface_x = x_max;
				interfacePtr->Set_left_right_TimeSequenceData(interfaceLoc, directionalBCTypeRightSide,
					bulks[num_bulks - 1], bRight,
					true, ts_bulkPropsInOut,
					interfaces[num_interfaces - 2], NULL, NULL);
				interfacePtr->sides_bulk_index[SDL] = num_bulks - 1;
				interfacePtr->sides_bulk_index[SDR] = -1;

				sd_numR = num_subdomains - 1;
				bulk_posR = num_bulks - 1 - subdomain_bulk_start_nos[sd_numR];
				osabci = &bulk_interfaces_subdomains[sd_numR];
				obsifi = &osabci->subdomain_bulk_segments[bulk_posR];
				//		obsifi->bulk_cntr = num_bulks - 1;
				//		obsifi->bulkPtr = bulks[num_bulks - 1];
				obsifi->interfaceRightOfBulk_cntr = index;
				obsifi->interfaceRightOfBulkPtr = interfacePtr;
				osabci->subdomain_interfaces[bulk_posR + 1] = interfacePtr;
			}
			if (hasSymAntiSym == false) // this interface cannot take fracture
			{
				interfacePtr->interfacePFs = new SL_Interface_Fracture_PF();
				interfacePtr->interfacePFs->CreateBonded_PF();
				interfacePtr->interfacePFs_Deletable = true;
			}
			else
				interfacePtr->Set_EF_Properties(interface_fracture_map[interfacePtr->interface_flag]);
			interfacePtr->Set1DOrtizType();
			min_domain_del_t = MIN(min_domain_del_t, interfacePtr->min_delT);
			max_domain_del_t = MAX(max_domain_del_t, interfacePtr->max_delT);
			interfacePtr->sz_subDomainNos = GetInterfaceBulkSide_Subdomains_RelIndices(interfacePtr, interfacePtr->subDomainNos, interfacePtr->relPos_wrt_subDomainStartPoints);
			if ((!g_low_disk_space) || (interfacePtr->interface_flag == g_interfaceFlags_IncImp[LI_INT]) || (interfacePtr->interface_flag == g_interfaceFlags_IncImp[RI_INT]))
				interfacePtr->Open_fixed_x_files_SL_OneInterfaceAllTimes(io_type_InterfaceRawFinalSln_AllTime_Print_4PP, io_type_InterfaceRawScalars_AllTime_Print_4PP, interfacePtr->subDomainNos[0], interfacePtr->subDomainNos[0]);

			if (g_SL_desc_data.tdLoadComputer != NULL)
			{
				if (i == 0)
				{
					interfacePtr->interface_flag = g_interfaceFlags_IncImp[LO_INT];
					if (isNeumannOrDirichlet && g_SL_desc_data.tdLoadSide == SDL)
						interfacePtr->interface_flag = g_interfaceFlags_IncImp[LI_INT];
				}
				else
				{
					interfacePtr->interface_flag = g_interfaceFlags_IncImp[RO_INT];
					if (isNeumannOrDirichlet && g_SL_desc_data.tdLoadSide == SDR)
						interfacePtr->interface_flag = g_interfaceFlags_IncImp[RI_INT];
				}
			}
		}
	}
	//////////////////////////// The interfaces that have bulk on the two sides

	// all the interior interiors that have two sides are set first
	// periodic:	 starts from interface 0 (right of bulk 0) to the very last. There is no interface on the left of bulk 0 -> This is represented by the very last interface at the very end (right of last bulk)
	// non-periodic: offset = 1, so, it'll skip one from either side as they are already taken case of above.
	unsigned int first_interior_interface = interface_offset, end_interior_interface = num_interfaces - 1 - interface_offset;
	unsigned int bulk_index_left, bulk_index_right, interface_index_left, interface_index_right;
	SL_Bulk_Properties *bulkLeft, *bulkRight;
	SL_OneInterfaceAllTimes *left_allTimes, *right_allTimes;
	bool ts_bulkPropsDeletableIn;

	for (unsigned int ii = first_interior_interface; ii <= end_interior_interface; ++ii)
	{
		bulk_index_left = ii - interface_offset;
		bulk_index_right = bulk_index_left + 1;
		interface_index_left = ii - 1;
		interface_index_right = ii + 1;
		if (isPeriodic)
		{
			if (ii == 0)
				interface_index_left = num_interfaces - 1;
			else if (ii == end_interior_interface)
			{
				interface_index_right = 0;
				bulk_index_right = 0;
			}
		}
		bulkLeft = bulks[bulk_index_left];
		bulkRight = bulks[bulk_index_right];
		left_allTimes = interfaces[interface_index_left];
		right_allTimes = interfaces[interface_index_right];
		interfacePtr = interfaces[ii];
		interfacePtr->sides_bulk_index[SDL] = bulk_index_left;
		interfacePtr->sides_bulk_index[SDR] = bulk_index_right;

		//// start: post-process members
		// PP.1 bulk on the left of the interface
		sd_numL = subdomainNo4AllBulks[bulk_index_left];
		bulk_posL = bulk_index_left - subdomain_bulk_start_nos[sd_numL];
		osabci = &bulk_interfaces_subdomains[sd_numL];
		obsifi = &osabci->subdomain_bulk_segments[bulk_posL];
		//		obsifi->bulk_cntr = bulk_index_left;
		//		obsifi->bulkPtr = bulkLeft;
		obsifi->interfaceRightOfBulk_cntr = ii;
		obsifi->interfaceRightOfBulkPtr = interfacePtr;
		osabci->subdomain_interfaces[bulk_posL + 1] = interfacePtr;

		// PP.2 bulk on the right side of the interface
		sd_numR = subdomainNo4AllBulks[bulk_index_right];
		bulk_posR = bulk_index_right - subdomain_bulk_start_nos[sd_numR];
		osabci = &bulk_interfaces_subdomains[sd_numR];
		obsifi = &osabci->subdomain_bulk_segments[bulk_posR];
		//		obsifi->bulk_cntr = bulk_index_right;
		//		obsifi->bulkPtr = bulkRight;
		obsifi->interfaceLeftOfBulk_cntr = ii;
		obsifi->interfaceLeftOfBulkPtr = interfacePtr;
		osabci->subdomain_interfaces[bulk_posR] = interfacePtr;
		//// end: post-process members

		// if either side is deletable, it means either side is inhomogeneous, so the inteface would be a new one created in the function below internally and need to be deleted
		ts_bulkPropsDeletableIn = (bulks_deletable[bulk_index_left] || bulks_deletable[bulk_index_right]);
		if (!ts_bulkPropsDeletableIn) // that is this an interface between two homogeneous interfaces
		{
			map<pair<GID, GID>, SL_Elastic_InterfaceProperties*>::iterator it;
			pair<GID, GID> pairLoc(bulkLeft->flag, bulkRight->flag);
			it = lr_IDS_2_ts_bulkProps.find(pairLoc);
			if (it == lr_IDS_2_ts_bulkProps.end())
				lr_IDS_2_ts_bulkProps[pairLoc] = NULL;

			interfacePtr->Set_left_right_TimeSequenceData(ilt_twoSided, NULL,
				bulkLeft, bulkRight,
				ts_bulkPropsDeletableIn, lr_IDS_2_ts_bulkProps[pairLoc],
				left_allTimes, right_allTimes,
				periodic_totalLength);
		}
		else
		{
			SL_Elastic_InterfaceProperties* ts_bulkPropsInOut = NULL;
			interfacePtr->Set_left_right_TimeSequenceData(ilt_twoSided, NULL,
				bulkLeft, bulkRight,
				ts_bulkPropsDeletableIn, ts_bulkPropsInOut,
				left_allTimes, right_allTimes,
				periodic_totalLength);
		}
		interfacePtr->sz_subDomainNos = GetInterfaceBulkSide_Subdomains_RelIndices(interfacePtr, interfacePtr->subDomainNos, interfacePtr->relPos_wrt_subDomainStartPoints);
		if (sd_numL != sd_numR)
		{
			interfacePtr->Open_fixed_x_files_SL_OneInterfaceAllTimes(iof_ascii, iof_ascii, interfacePtr->subDomainNos[0], interfacePtr->subDomainNos[1]);
			if (g_SL_desc_data.tdLoadComputer != NULL)
			{
				if (interfacePtr->subDomainNos[0] == 0)
					interfacePtr->interface_flag = g_interfaceFlags_IncImp[LI_INT];
				else if (interfacePtr->subDomainNos[1] == (num_subdomains - 1))
					interfacePtr->interface_flag = g_interfaceFlags_IncImp[RI_INT];

				if (g_SL_desc_data.tdLoadType == lmt_Incident)
				{
					if (g_SL_desc_data.tdLoadSide == SDL)
					{
						if (interfacePtr->interface_flag == g_interfaceFlags_IncImp[LI_INT])
							interfacePtr->incidentSide = ilt_left;
					}
					else if (g_SL_desc_data.tdLoadSide == SDR)
					{
						if (interfacePtr->interface_flag == g_interfaceFlags_IncImp[RI_INT])
							interfacePtr->incidentSide = ilt_right;
					}
				}
			}
		}
		else if(io_type_InterfaceRawFinalSln_AllTime_Print_4PP != iof_none)
		{
			int step = numSpaceStep_InterfaceRawFinalSlnScalars_AllTime_Print_4PP;
			if (step < 0)
			{
				unsigned int st = subdomain_bulk_start_nos[sd_numL], en = subdomain_bulk_start_nos[sd_numL + 1], szz = en - st;
				step = szz / -step;
				if (step == 0)
					step = 1;
			}
			
			if ((step > 0) && ((interfacePtr->interface_pos - st) % step == 0)) // bulk_posL
				interfacePtr->Open_fixed_x_files_SL_OneInterfaceAllTimes(io_type_InterfaceRawFinalSln_AllTime_Print_4PP, io_type_InterfaceRawScalars_AllTime_Print_4PP);
		}
		interfacePtr->Set1DOrtizType();
		min_domain_del_t = MIN(min_domain_del_t, interfacePtr->min_delT);
		max_domain_del_t = MAX(max_domain_del_t, interfacePtr->max_delT);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////  post-process
	for (unsigned int si = 0; si < num_subdomains; ++si)
	{
		OneSubdomain_All_bulksConnectivityInfo* osabci = &bulk_interfaces_subdomains[si];
		st = subdomain_bulk_start_nos[si];
		en = subdomain_bulk_start_nos[si + 1];
		sz = en - st;
		for (unsigned int j = 0; j <= sz; ++j)
			osabci->subdomain_interface_xs[j] = osabci->subdomain_interfaces[j]->interface_x;
	}
	if (isPeriodic)  // to avoid this being equal to 
		bulk_interfaces_subdomains[0].subdomain_interface_xs[0] = x_min;
}

void Domain_All_Interfaces_All_Times::Generate_subdomain_nos_for_all_bulks(vector<int>& subdomainNo4AllBulks)
{
	subdomainNo4AllBulks.resize(num_bulks);
	unsigned int st, en;
	for (unsigned int si = 0; si < num_subdomains; ++si)
	{
		st = subdomain_bulk_start_nos[si];
		en = subdomain_bulk_start_nos[si + 1];
		for (unsigned int i = st; i < en; ++i)
			subdomainNo4AllBulks[i] = si;
	}
}

bool Domain_All_Interfaces_All_Times::OpenFiles_RawData_OneTimeAllSpatialPoints(unsigned int timeIndex, double time, unsigned int numTimes)
{
	if ((numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP == 0) || 
		((timeIndex % numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP != 0) && ((timeIndex != numTimes)))		)
		return false;
	// opening files

	for (unsigned int si = 0; si < num_subdomains; ++si)
	{
		string outName;
		string ext;
		bool genFile = getExt(io_type_InterfaceRawFinalSln_AllSpace_Print_4PP, ext);
		if (genFile)
		{
			string specificName = "InterfaceRawFinalSln";
			GetSubdomainIndexed_TimeIndexed_FileName(outName, si, timeIndex, specificName, ext);
			if (io_type_InterfaceRawFinalSln_AllSpace_Print_4PP == iof_ascii)
				outFinalSln_fixed_time[si] = new fstream(outName.c_str(), ios::out);
			else
				outFinalSln_fixed_time[si] = new fstream(outName.c_str(), ios::out | ios::binary);
		}
		genFile = getExt(io_type_InterfaceRawScalars_AllSpace_Print_4PP, ext);

		if (io_type_InterfaceRawScalars_AllSpace_Print_4PP)// && (timeIndex > 0))
		{
			string specificName = "InterfaceRawScalars";
			GetSubdomainIndexed_TimeIndexed_FileName(outName, si, timeIndex, specificName, ext);
			if (io_type_InterfaceRawScalars_AllSpace_Print_4PP == iof_ascii)
				outScalars_fixed_time[si] = new fstream(outName.c_str(), ios::out);
			else
				outScalars_fixed_time[si] = new fstream(outName.c_str(), ios::out | ios::binary);
			
		}
	}
	return true;
}

void Domain_All_Interfaces_All_Times::Print__RawData_OneTimeAllSpatialPoints(SL_OneInterfaceAllTimes* interfacePtr, SLInterfaceCalculator & slic)
{
	for (unsigned int ind = 0; ind < interfacePtr->sz_subDomainNos; ++ind)
	{
		unsigned int subDomainNo = interfacePtr->subDomainNos[ind];
		unsigned int relPos_wrt_subDomainStartPoints = interfacePtr->relPos_wrt_subDomainStartPoints[ind];
		slic.Output_SLInterfaceCalculator(true, io_type_InterfaceRawFinalSln_AllSpace_Print_4PP, io_type_InterfaceRawScalars_AllSpace_Print_4PP, interfacePtr->interface_x, outScalars_fixed_time[subDomainNo], outFinalSln_fixed_time[subDomainNo], NULL, NULL);
	}
}

void Domain_All_Interfaces_All_Times::Close_Files_RawData_OntTimeAllSpatialPoints()
{
	if (io_type_InterfaceRawFinalSln_AllSpace_Print_4PP == iof_none)
		return;
	for (unsigned int i = 0; i < outScalars_fixed_time.size(); ++i)
		if (outScalars_fixed_time[i] != NULL)
		{
			delete outScalars_fixed_time[i];
			outScalars_fixed_time[i] = NULL;
		}
	for (unsigned int i = 0; i < outFinalSln_fixed_time.size(); ++i)
		if (outFinalSln_fixed_time[i] != NULL)
		{
			delete outFinalSln_fixed_time[i];
			outFinalSln_fixed_time[i] = NULL;
		}
}

unsigned int Domain_All_Interfaces_All_Times::GetInterfaceBulkSide_Subdomains_RelIndices(SL_OneInterfaceAllTimes* interfacePtr, vector<unsigned int>& subDomainNos, vector<unsigned int>& relPos_wrt_subDomainStartPoints)
{
	subDomainNos.clear();
	relPos_wrt_subDomainStartPoints.clear();

	int subdomain_index = interfacePtr->sides_bulk_index[SDL], subdomain_no = -1, subdomain_no_r;
	if (subdomain_index >= 0)
	{
		subdomain_no = subdomainNo4AllBulks[subdomain_index];
		subDomainNos.push_back(subdomain_no);
		relPos_wrt_subDomainStartPoints.push_back(subdomain_index - subdomain_bulk_start_nos[subdomain_no]);
	}
	subdomain_index = interfacePtr->sides_bulk_index[SDR];
	if (subdomain_index >= 0)
	{
		subdomain_no_r = subdomainNo4AllBulks[subdomain_index];
		if (subdomain_no_r != subdomain_no)
		{
			subDomainNos.push_back(subdomain_no_r);
			relPos_wrt_subDomainStartPoints.push_back(subdomain_index - subdomain_bulk_start_nos[subdomain_no_r]);
		}
	}
	return relPos_wrt_subDomainStartPoints.size();
}

void Domain_All_Interfaces_All_Times::Set_InitialCondition_step()
{
	bool printRaw = OpenFiles_RawData_OneTimeAllSpatialPoints(0, 0.0, 1);

	for (unsigned int i = 0; i < num_interfaces; ++i)
	{
		SL_OneInterfaceAllTimes* interfacePtr = interfaces[i];
		SLInterfaceCalculator slic;
		interfacePtr->InitialStep_Use_IC(slic);
		if (printRaw)
			Print__RawData_OneTimeAllSpatialPoints(interfacePtr, slic);
	}
	if (printRaw)
		Close_Files_RawData_OntTimeAllSpatialPoints();
}

//fstream lgo("_log_out.txt", ios::out);
int Domain_All_Interfaces_All_Times::TimeStepsNonAdaptive()
{
	double timeStep = g_slf_conf->uniform_del_t;
	double maxTime = g_slf_conf->terminate_run_target_time;

//	timeStep = maxTime / ceil(maxTime / timeStep);
//	g_slf_conf->uniform_del_t = timeStep;
//	g_slf_conf->inv_uniform_del_t = 1.0 / g_slf_conf->uniform_del_t;
	double current_min_time = 0.0;
	double maxTimewTol = maxTime - 1e-6 * timeStep;
	bool accept_point;
	int timeIndex = 0;

	bool uniform_delt = true;
	unsigned int numTimes = (int)ceil(maxTime / timeStep - 1e-6);
	if (numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP < 0)
		numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP = MAX(1, numTimes / -numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP);

	if (numTimeStep_BulkInterfacePoints_Print_4PP < 0)
		numTimeStep_BulkInterfacePoints_Print_4PP = MAX(1, numTimes / -numTimeStep_BulkInterfacePoints_Print_4PP);
	if (numTimeStep_Interface_DSU_Fragment_Print_4PP < 0)
		numTimeStep_Interface_DSU_Fragment_Print_4PP = MAX(1, numTimes / -numTimeStep_Interface_DSU_Fragment_Print_4PP);

	for (unsigned int si = 0; si < num_subdomains; ++si)
	{
		bulk_interfaces_subdomains[si].OneSubdomain_All_bulksConnectivityInfo_Initialize();
		string fileName;
		string specificName = "keyParameters";
		bool addUnderline = true;
	
		GetSubdomainIndexed_TimeIndexed_FileName(fileName, si, -1, specificName, "txt", addUnderline);

		fstream out(fileName.c_str(), ios::out);
		out << setprecision(22);
		out << "isPeriodic\t" << isPeriodic << '\n';
		out << "maxTime\t" << maxTime << '\n';
		out << "timeStep\t" << timeStep << '\n';
		out << "totalTimeSteps\t" << ceil(maxTimewTol / timeStep - 1e-16) << '\n';
		out << "numTimeStep_Interface_DSU_Fragment_Print_4PP\t" << numTimeStep_Interface_DSU_Fragment_Print_4PP << '\n';
		out << "numTimeStep_BulkInterfacePoints_Print_4PP\t" << numTimeStep_BulkInterfacePoints_Print_4PP << '\n';
		out << "numSpatialSubsegments_BulkInterfacePoints_Print_4PP\t" << numSpatialSubsegments_BulkInterfacePoints_Print_4PP << '\n';
		out << "numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP\t" << numTimeStep_InterfaceRawFinalSlnScalars_AllSpace_Print_4PP << '\n';
		double sigmaCScale = 1.0, deltaCScale = 1.0, energyCScale = 1.0;
		map<GID, SL_Interface_Fracture_PF*>::const_iterator it = interface_fracture_map.find(1);
		if (it != interface_fracture_map.end())
			it->second->Get_sigmaC_deltaC_phiC_scales(sigmaCScale, deltaCScale, energyCScale);
		double EScale = 1.0, rhoScale = 1.0, dampingScale = 0;
		map<GID, SL_Bulk_Properties*>::const_iterator itb = bulk_elastic_map.find(1);
		if (itb != bulk_elastic_map.end())
		{ 
			EScale = itb->second->E_iso;
			rhoScale = itb->second->rho;
#if HAVE_SOURCE_ORDER0_q
			dampingScale = itb->second->D_vv;
#endif
		}
		out << "loadTimeScale\t" << g_SL_desc_data.GetLoadingTimeScale() << '\n';
		out << "EScale\t" << EScale << "\trhoScale\t" << rhoScale << "\tdampingScale\t" << dampingScale << '\n';
		out << "sigmaCScale \t" << sigmaCScale << "\tsigmaCScale\t" << deltaCScale << "\tenergyCScale\t" << energyCScale << '\n';
		bulk_interfaces_subdomains[si].PrintIndicesLengthsKeyRunParameters(out);
	}

	///// Initializing post-process class
	if (do_space_spacetime_PP)
	{
		postProcessing_subdomains.resize(num_subdomains);
		for (unsigned int si = 0; si < num_subdomains; ++si)
		{
			postProcessing_subdomains[si].Initialize_Subdomain_spacetime_pp_data(numTimes, &bulk_interfaces_subdomains[si],
				uniform_delt, numTimeStep_BulkInterfacePoints_Print_4PP, numTimeStep_Interface_DSU_Fragment_Print_4PP, numSpatialSubsegments_BulkInterfacePoints_Print_4PP, useRepeatedSimpsonRuleForHigherOrders_4PP);
			postProcessing_subdomains[si].AddComputeTimeStep(timeIndex, current_min_time);
		}
	}

	while (current_min_time < maxTimewTol)
	{
		g_time = current_min_time;
//		lgo << "ti" << timeIndex << '\n';
//		lgo.flush();
		bool printRaw = OpenFiles_RawData_OneTimeAllSpatialPoints(timeIndex, current_min_time, numTimes);
		DBF(dbout << "\n\n\n\nTimeIndex" << timeIndex << "\tcurent_min_time\t" << current_min_time << '\n';);
		for (unsigned int i = 0; i < num_interfaces; ++i)
		{
			DBF(b_db_p = (i == 2);); // 3
//			lgo << "i" << i << ' ';
//			lgo.flush();
			SL_OneInterfaceAllTimes* interfacePtr = interfaces[i];
			SLInterfaceCalculator slic;
			AdaptivityS as = interfacePtr->NonInitialStep(timeStep, accept_point, maxTime, slic);
			if (printRaw && accept_point)
				Print__RawData_OneTimeAllSpatialPoints(interfacePtr, slic);

			AdaptivityF a_flag = as.get_a_flag();
//			if ((maxTime >= g_slf_conf->terminate_run_target_time) || (a_flag == a_terminate_run_correctly))
//				return 1;
			if (a_flag == a_terminate_run_prematurely)
				return 0;
		}
		if (printRaw)
			Close_Files_RawData_OntTimeAllSpatialPoints();
		current_min_time += timeStep;
		++timeIndex;

		if (do_space_spacetime_PP)
			for (unsigned int si = 0; si < num_subdomains; ++si)
				postProcessing_subdomains[si].AddComputeTimeStep(timeIndex, current_min_time);

		if (timeIndex % 100 == 0)
			cout << timeIndex << '\t' << current_min_time << '\n';
	}
	return 1;
}

int MAIN_Domain(string config1, int serialNumberIn, string configBC, string config_TS_Adapt)
{
	string configDomain;
	string buf;
	if (configBC == "none")
	{
		fstream in(config1.c_str(), ios::in);
		if (!in.is_open())
		{
			cout << "config1\t" << config1 << '\n';
			THROW("Cannot open the main input config file\n");
		}
		READ_NSTRING(in, buf, configDomain);
		READ_NSTRING(in, buf, configBC);
		READ_NSTRING(in, buf, config_TS_Adapt);
	}
	else
		configDomain = config1;

	if (configBC != "default")
	{
		if (configBC == "infile")
			configBC = config1;
		g_SL_desc_data.Read(configBC);
	}
	if (config_TS_Adapt != "default")
	{
		if (config_TS_Adapt == "infile")
			config_TS_Adapt = config1;
		g_slf_conf->Read(config_TS_Adapt);
	}
	else
		g_slf_conf->Initialize_SLFractureGlobal_Configuration_After_Reading();

	// 1. Reading the domain
	if (configDomain == "infile")
		configDomain = config1;
	Domain_All_Interfaces_All_Times domain;
	domain.Read_Initialize(configDomain, serialNumberIn);

	// 2. Upate time steps of time step and adaptive config based on min time step of domain
	g_slf_conf->UpdateTimeScales(domain.min_domain_del_t, domain.max_domain_del_t);

	/// debug print
#if PRINT_DOMAIN_CONFIG
	string fileName = g_prefileName + "/" + "__domainConfig.txt";
	fstream out(fileName.c_str(), ios::out);
	domain.Print(out);
#endif

	// 3. Step 0 -> set initial condition & solving the domain
	int successMode = domain.Main_Domain_Solution();
	return successMode;
}

void Configure_sfcm_sfcm_gen()
{
	if (!sfcm.success)
		return;
	string key;
	double value;
	map<string, string>* mpPtr;
	if (sfcm_gen.specificProblemName == "axt")
	{
		double llc = -1, la = 0, ldelc = -1;
		key = "llc";
		if (Find_Version_Value(key, value, mpPtr))
			llc = value;
		key = "la";
		if (Find_Version_Value(key, value, mpPtr))
			la = value;
		key = "ldelc";
		if (Find_Version_Value(key, value, mpPtr))
			ldelc = value;
		sfcm.cfl_factor = 1.0;
		sfcm.tFinal = 10.0;
		sfcm.number_of_elements = 1024;
		if ((ldelc < -1) || (la > 2) || (llc < -2.51))
		{
			sfcm.number_of_elements = 8192;
			THROW("generate appropriate input meshes, update the statements below\n");
		}
		double tol = 1e-7;
		if (ldelc >= -1.01) // decently large correlation length
		{
			if (fabs(la + 3) < tol) // loading rate of -3
				sfcm.tFinal = 1280;
			if (fabs(la + 2.5) < tol) // loading rate of -3
				sfcm.tFinal = 640;
			else if (fabs(la + 2) < tol)
				sfcm.tFinal = 200;
			else if (fabs(la + 1.5) < tol)
				sfcm.tFinal = 128;
			else if (fabs(la + 1) < tol)
			{
				sfcm.tFinal = 100;
				sfcm.cfl_factor = 0.256;
			}
			else if (fabs(la + 0.5) < tol)
			{
				sfcm.tFinal = 64;
				sfcm.cfl_factor = 0.256;
			}
			else if (fabs(la) < tol)
			{
				sfcm.tFinal = 10;
				sfcm.cfl_factor = 0.128;
			}
			else if (fabs(la - 1) < tol)
			{
				sfcm.tFinal = 1;
				sfcm.cfl_factor = 0.0128;
			}
			else if (fabs(la - 1.5) < tol)
			{
				sfcm.tFinal = 6;
				sfcm.cfl_factor = 0.0128;
			}
			else if (fabs(la - 2) < tol)
			{
				sfcm.tFinal = 4;
				sfcm.cfl_factor = 0.0128;
//				sfcm.cfl_factor = 0.0064;
			}
		}
		else
		{
			THROW("add these options later\n");
		}
	}

}