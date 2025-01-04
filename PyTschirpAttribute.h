/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#pragma once

#include "Patch.h"

#ifdef _MSC_VER
#pragma warning ( push )
#pragma warning ( disable: 4100 )
#endif
#include <pybind11/pybind11.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class PyTschirpAttribute {
public:
	PyTschirpAttribute(std::shared_ptr<midikraft::Patch> patch, std::string const &param);
	PyTschirpAttribute(std::shared_ptr<midikraft::Patch> patch, std::string const &param, int targetLayerNo);

	void set(int value);
	void set(std::vector<int> data);

	pybind11::object get() const;

	std::string asText() const;

	// Bindings not for python
	std::shared_ptr <midikraft::SynthParameterDefinition> def();

private:
	std::shared_ptr<midikraft::SynthParameterDefinition> defByName(std::string const &name) const;

	std::shared_ptr<midikraft::Patch> patch_;
	std::shared_ptr<midikraft::SynthParameterDefinition> def_;
};

