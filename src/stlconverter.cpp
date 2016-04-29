#include "stlconverter.h"

namespace StlConverter {

	/**
	* Converter class to go from a list of triangle to a .stl file. 
	* @param tris the list of triangles to use. 
	* @param filepath the full path to write the output to. 
	*/
	Converter::Converter(std::vector<MeshUtil::TriangleF> tris, std::string filepath) :
		 path(filepath){
		this->data.reserve(tris.size());
		this->data = tris;
	}

	/**
	* Performs the conversion.
	*/
	void Converter::convert() {
		//open output and write .stl as ascii. 
		std::ofstream out;
		out.open(path);
		out << "solid ascii";
		for (auto t : data) {
			auto nor = t.normal();
			out << "  facet normal " << nor.x << ' ' << nor.y << ' ' << nor.z << '\n';
			out << "    outer loop" << '\n';
			out << "      vertex " << t.p1.x << ' ' << t.p1.y << ' ' << t.p1.z << '\n';
			out << "      vertex " << t.p2.x << ' ' << t.p2.y << ' ' << t.p2.z << '\n';
			out << "      vertex " << t.p3.x << ' ' << t.p3.y << ' ' << t.p3.z << '\n';
			out << "    end loop" << '\n';
			out << "  end facet" << '\n';
		}	
		out << "end solid";
		out.close();
	}
}