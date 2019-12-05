/*
   Copyright (c) 2019 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#pragma once

#include "EditBufferCapability.h"
#include "SimpleDiscoverableDevice.h"
#include "MidiRequest.h"

template<typename SYNTH, typename TSCHIRP>
class PyTschirpSynth {
public:
	PyTschirpSynth() {
		synth_ = std::make_shared<SYNTH>();
	}

	void detect() {
		std::vector<std::shared_ptr<midikraft::SimpleDiscoverableDevice>> list;
		list.push_back(synth_);
		midikraft::AutoDetection autodetection;
		autodetection.autoconfigure(list);
	}

	bool detected() {
		return synth_->channel().isValid();
	}

	std::string location() const {
		auto result = String("MIDI IN: ") + synth_->midiInput() + String(", MIDI OUT: ") + synth_->midiOutput();
		return result.toStdString();
	}

	TSCHIRP editBuffer() {
		if (!detected()) {
			throw std::runtime_error("PyTschirp: Synth hasn't been detected yet - run detect() first and check if it worked");
		}

		// Let's see if this is possible
		auto editBufferCapability = std::dynamic_pointer_cast<midikraft::EditBufferCapability>(synth_);
		if (editBufferCapability) {
			// Block until we get the edit buffer back from the synth!
			auto request = editBufferCapability->requestEditBufferDump();
			midikraft::MidiRequest requester(synth_->midiOutput(), request, [editBufferCapability](MidiMessage const &message) {
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
			return TSCHIRP(patches[0], synth_);
		}
		else {
			std::cerr << "The " << synth_->getName() << " has no capability to recall the edit buffer, failed." << std::endl;
			throw std::runtime_error("PyTschirp: No edit buffer capability in device");
		}
	}

private:
	std::shared_ptr<SYNTH> synth_;
};


