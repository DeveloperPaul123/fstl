#ifndef STLCONVERTER_H
#define STLCONVERTER_H

#include "mesh_util_core.h"
#include <iostream>
#include <fstream>

namespace StlConverter {

	class Converter { 
	public:
		Converter(std::vector<MeshUtil::TriangleF> tris, std::string filepath);
		void convert();

	private:
		std::string path;
		std::vector<MeshUtil::TriangleF> data;

	};
}
#endif