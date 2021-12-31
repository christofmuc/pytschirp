/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#include "PyTschirpSynth.h"

#include "Capability.h"

#include "SimpleDiscoverableDevice.h"
#include "ProgramDumpCapability.h"

#include "EditBufferCapability.h"
#include "MidiRequest.h"
#include "Sysex.h"
#include "Librarian.h"


PyTschirpSynth::PyTschirpSynth(std::shared_ptr<midikraft::Synth> synth)
{
	synth_ = synth;
}

void PyTschirpSynth::detect()
{
	std::vector<std::shared_ptr<midikraft::SimpleDiscoverableDevice>> list;
	list.push_back(std::dynamic_pointer_cast<midikraft::SimpleDiscoverableDevice>(synth_));
	midikraft::AutoDetection autodetection;
	autodetection.autoconfigure(list, nullptr);
}

bool PyTschirpSynth::detected()
{
	return channel().isValid();
}

std::string PyTschirpSynth::location() const
{
	auto result = String("MIDI IN: ") + String(midiInput().identifier) + String(", MIDI OUT: ") + String(midiOutput().identifier);
	return result.toStdString();
}

PyTschirp PyTschirpSynth::editBuffer()
{
	if (!detected()) {
		throw std::runtime_error("PyTschirp: Synth hasn't been detected yet - run detect() first and check if it worked");
	}

	// Let's see if this is possible
	auto editBufferCapability = midikraft::Capability::hasCapability<midikraft::EditBufferCapability>(synth_);
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

std::vector<PyTschirp> PyTschirpSynth::loadSysex(std::string const &filename)
{
	auto midimessages = Sysex::loadSysex(filename);
	auto patches = synth_->loadSysex(midimessages);

	std::vector<PyTschirp> result;
	for (auto patch : patches) {
		result.emplace_back(patch, synth_);
	}
	return result;
}

void PyTschirpSynth::saveSysex(std::string const &filename, std::vector <PyTschirp> &patches)
{
	auto pdc = midikraft::Capability::hasCapability<midikraft::ProgramDumpCabability>(synth_);
	if (pdc) {
		std::vector<MidiMessage> result;
		int place = 0;
		for (auto tschirp : patches) {
			auto m = pdc->patchToProgramDumpSysex(tschirp.patchPtr(), MidiProgramNumber::fromZeroBase(place++));
			std::copy(m.cbegin(), m.cend(), std::back_inserter(result));
		}
		Sysex::saveSysex(filename, result);
	}
	else {
		throw std::runtime_error("PyTschirp: Synth has not implemented the ProgramDumpCapability, consider saving the patches one by one with the saveEditBuffer() function");
	}
}

void PyTschirpSynth::saveEditBuffer(std::string const &filename, PyTschirp &patch)
{
	auto ebc = midikraft::Capability::hasCapability<midikraft::EditBufferCapability>(synth_);
	if (ebc) {
		auto midiMessages = ebc->patchToSysex(patch.patchPtr());
		Sysex::saveSysex(filename, midiMessages);
	}
	else {
		throw std::runtime_error("PyTschirp: Synth has not implemented the EditBufferCapability, cannot save the patch as edit buffer sysex");
	}
}

void PyTschirpSynth::getGlobalSettings()
{
	auto discoverableDevice = std::dynamic_pointer_cast<midikraft::SimpleDiscoverableDevice>(synth_);
	if (!discoverableDevice) {
		//TODO - this shouldn't happen too often?
		return;

	}
	std::vector<midikraft::SynthHolder> synths({ midikraft::SynthHolder(discoverableDevice, Colours::black) });
	midikraft::Librarian librarian(synths);
	bool done = false;

	auto dataFileLoad = midikraft::Capability::hasCapability<midikraft::DataFileLoadCapability>(synth_);
	if (dataFileLoad) {
		//TODO - how to determine the data file type for the global settings? I don't think this will work. 
		/*librarian.startDownloadingSequencerData(midikraft::MidiController::instance()->getMidiOutput(midiOutput()), dataFileLoad.get(), 0, nullptr, [this, &done](std::vector<std::shared_ptr<midikraft::DataFile>>) {
			done = true;
		});*/
	}
	midikraft::MidiRequest::blockUntilTrue([&done]() { return done;  }, 2000);
}

juce::MidiDeviceInfo PyTschirpSynth::midiInput() const
{
	auto midiLocation = midikraft::Capability::hasCapability<midikraft::MidiLocationCapability>(synth_);
	return midiLocation ? midiLocation->midiInput() : MidiDeviceInfo();
}

juce::MidiDeviceInfo PyTschirpSynth::midiOutput() const
{
	auto midiLocation = midikraft::Capability::hasCapability<midikraft::MidiLocationCapability>(synth_);
	return midiLocation ? midiLocation->midiOutput() : MidiDeviceInfo();
}

MidiChannel PyTschirpSynth::channel() const
{
	auto midiLocation = midikraft::Capability::hasCapability<midikraft::MidiLocationCapability>(synth_);
	return midiLocation ? midiLocation->channel() : MidiChannel::invalidChannel();
}
