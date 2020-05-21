/*
   Copyright (c) 2019 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#pragma once

#include "EditBufferCapability.h"
#include "SimpleDiscoverableDevice.h"
#include "MidiRequest.h"
#include "Sysex.h"
#include "Librarian.h"

class PyTschirpSynth {
public:
	PyTschirpSynth(std::shared_ptr<midikraft::Synth> synth) {
		synth_ = synth;
	}

	void detect() {
		std::vector<std::shared_ptr<midikraft::SimpleDiscoverableDevice>> list;
		list.push_back(std::dynamic_pointer_cast<midikraft::SimpleDiscoverableDevice>(synth_));
		midikraft::AutoDetection autodetection;
		autodetection.autoconfigure(list, nullptr);
	}

	bool detected() {
		return channel().isValid();
	}

	std::string location() const {
		auto result = String("MIDI IN: ") + String(midiInput()) + String(", MIDI OUT: ") + String(midiOutput());
		return result.toStdString();
	}

	PyTschirp editBuffer() {
		if (!detected()) {
			throw std::runtime_error("PyTschirp: Synth hasn't been detected yet - run detect() first and check if it worked");
		}

		// Let's see if this is possible
		auto editBufferCapability = std::dynamic_pointer_cast<midikraft::EditBufferCapability>(synth_);
		if (editBufferCapability) {
			// Block until we get the edit buffer back from the synth!
			auto request = editBufferCapability->requestEditBufferDump();
			midikraft::MidiRequest requester(midiOutput(), request, [editBufferCapability](MidiMessage const &message) {
				return editBufferCapability->isEditBufferDump(message);
			});

			auto editBufferMessage = requester.blockForReply();

			auto patches = synth_->loadSysex({ editBufferMessage });
			if (patches.empty()) {
				throw std::runtime_error("PyTschirp: Failed to create edit buffer from reply, program error!");
			}
			if (!patches[0]) {
				throw std::runtime_error("PyTschirp: Failed to parse edit buffer, program error!");
			}
			
			return PyTschirp(patches[0], synth_);
		}
		else {
			std::cerr << "The " << synth_->getName() << " has no capability to recall the edit buffer, failed." << std::endl;
			throw std::runtime_error("PyTschirp: No edit buffer capability in device");
		}
	}

	std::vector<PyTschirp> loadSysex(std::string const &filename) {
		auto midimessages = Sysex::loadSysex(filename);
		auto patches = synth_->loadSysex(midimessages);
		
		std::vector<PyTschirp> result;
		for (auto patch : patches) {
			result.emplace_back(patch, synth_);
		}
		return result;
	}

	void saveSysex(std::string const &filename, std::vector <PyTschirp> &patches) {
		auto pdc = std::dynamic_pointer_cast<midikraft::ProgramDumpCabability>(synth_);
		if (pdc) {
			std::vector<MidiMessage> result;
			for (auto tschirp : patches) {
				auto m = pdc->patchToProgramDumpSysex(*tschirp.patchPtr());
				std::copy(m.cbegin(), m.cend(), std::back_inserter(result));
			}
			Sysex::saveSysex(filename, result);
		}
		else {
			throw std::runtime_error("PyTschirp: Synth has not implemented the ProgramDumpCapability, consider saving the patches one by one with the saveEditBuffer() function");
		}
	}

	void saveEditBuffer(std::string const &filename, PyTschirp &patch) {
		auto ebc = std::dynamic_pointer_cast<midikraft::EditBufferCapability>(synth_);
		if (ebc) {
			auto midiMessages = ebc->patchToSysex(*patch.patchPtr());
			Sysex::saveSysex(filename, midiMessages);
		}
		else {
			throw std::runtime_error("PyTschirp: Synth has not implemented the EditBufferCapability, cannot save the patch as edit buffer sysex");
		}
	}

	void getGlobalSettings() {
		auto discoverableDevice = std::dynamic_pointer_cast<midikraft::SimpleDiscoverableDevice>(synth_);
		if (!discoverableDevice) {
			//TODO - this shouldn't happen too often?
			return;

		}
		std::vector<midikraft::SynthHolder> synths({ midikraft::SynthHolder(discoverableDevice, Colours::black) });
		midikraft::Librarian librarian(synths);
		bool done = false;

		auto dataFileLoad = std::dynamic_pointer_cast<midikraft::DataFileLoadCapability>(synth_);
		if (dataFileLoad) {
			//TODO - how to determine the data file type for the global settings? I don't think this will work. 
			librarian.startDownloadingSequencerData(midikraft::MidiController::instance()->getMidiOutput(midiOutput()), dataFileLoad.get(), 0, nullptr, [this, &done](std::vector<std::shared_ptr<midikraft::DataFile>>) {
				done = true;
			});
		}
		midikraft::MidiRequest::blockUntilTrue([&done]() { return done;  }, 2000);
	}

private:
	std::string midiInput() const {
		auto midiLocation = std::dynamic_pointer_cast<midikraft::MidiLocationCapability>(synth_);
		return midiLocation ? midiLocation->midiInput() : "invalid";
	}

	std::string midiOutput() const {
		auto midiLocation = std::dynamic_pointer_cast<midikraft::MidiLocationCapability>(synth_);
		return midiLocation ? midiLocation->midiOutput() : "invalid";
	}

	MidiChannel channel() const {
		auto midiLocation = std::dynamic_pointer_cast<midikraft::MidiLocationCapability>(synth_);
		return midiLocation ? midiLocation->channel() : MidiChannel::invalidChannel();
	}

	std::shared_ptr<midikraft::Synth> synth_;
};


