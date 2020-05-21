/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#pragma once

#include "SynthParameterDefinition.h"
#include "Synth.h"
#include "Patch.h"
#include "AutoDetection.h"

#include <pybind11/pybind11.h>

#include "PyTschirpAttribute.h"

class PyTschirp {	
public:
	PyTschirp(std::shared_ptr<midikraft::Patch> patch);
	PyTschirp(std::shared_ptr<midikraft::DataFile> p, std::weak_ptr<midikraft::Synth> synth);

	PyTschirpAttribute get_attr(std::string const &attrName);

	void set_attr(std::string const &name, int value);
	void set_attr(std::string const &name, std::vector<int> const &value);

	std::string getName();
	void setName(std::string const &newName);

	PyTschirp layer(int layerNo);

	std::string toText();

	std::vector<std::string> parameterNames();

	//! Use this at your own risk
	std::shared_ptr<midikraft::Patch> patchPtr();

private:
	// Private constructor to create a layer accessing Tschirp
	PyTschirp(std::shared_ptr<midikraft::Patch> p, std::weak_ptr<midikraft::Synth> synth, int layerNo);

	std::string underscoreToSpace(std::string const &input);

	std::string midiInput();
	std::string midiOutput();
	bool isChannelValid() const;

	std::shared_ptr<midikraft::Patch> patch_;
	std::weak_ptr<midikraft::Synth> synth_;
	int layerNo_ = -1; // -1 means no layer is selected, access the whole patch. Else, this is the layer number this Tschirp represents
};

