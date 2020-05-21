/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#pragma once

#include "Synth.h"

#include "PyTschirpPatch.h"

class PyTschirpSynth {
public:
	PyTschirpSynth(std::shared_ptr<midikraft::Synth> synth);

	void detect();
	bool detected();

	std::string location() const;

	PyTschirp editBuffer();

	std::vector<PyTschirp> loadSysex(std::string const &filename);
	void saveSysex(std::string const &filename, std::vector <PyTschirp> &patches);

	void saveEditBuffer(std::string const &filename, PyTschirp &patch);

	void getGlobalSettings();

private:
	std::string midiInput() const;
	std::string midiOutput() const;
	MidiChannel channel() const;

	std::shared_ptr<midikraft::Synth> synth_;
};

