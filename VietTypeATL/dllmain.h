// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

// dllmain.h : Declaration of module class.

class CVietTypeATLModule : public ATL::CAtlDllModuleT< CVietTypeATLModule >
{
public :
	DECLARE_LIBID(LIBID_VietTypeATLLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_VIETTYPEATL, "{4914ef26-8318-4608-aa14-4f7cbb6229e5}")
};

extern class CVietTypeATLModule _AtlModule;
