/*******************************************************************************
* ASTex:                                                                       *
* Copyright (C) IGG Group, ICube, University of Strasbourg, France             *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: https://astex-icube.github.io                                      *
* Contact information: astex@icube.unistra.fr                                  *
*                                                                              *
*******************************************************************************/



#include "quilting.h"

int main(int argc, char** argv)
{

	Quilting q;

	std::string fname;
	if (argc == 2)
		fname = std::string(argv[1]);
	else
		fname = TEMPO_PATH+"quilting_input.png";

	q.loadInput(fname);
	q.computeFittingTilesPathCut(2048,2048,64,16,5,4096);

	std::string out_name = ASTex::IO::remove_ext(fname)+"quilting_out.png";
	q.saveOutput(out_name);


  return EXIT_SUCCESS;
}

