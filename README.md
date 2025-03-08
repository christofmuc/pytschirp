# Introduction

This project builds Python language bindings for MIDI-capable synthesizers, to allow you to program these synthesizers in, you guessed it, Python. 

*Real synthesizer programming.*

For the time being, currently only the [Sequential](https://www.sequential.com/) Prophet Rev2 synthesizer is supported, but many more synths are on their way, let us know if you have a specific device you want supported.

# Installing

The easiest way to test the functionality is to head over to the little UI program we made called [PyTschirper](https://github.com/christofmuc/PyTschirper) here on github. When you install that (setup available for Windows), you get an experimental IDE so you can try it out.

The PyTschirper setup also installs a Windows Python binary module that you can import, see below "Usage" for details.

## Building

This is a git submodule that is intended to be used within a larger environment, therefore we don't provide build instructions here. To check out how it works, look at the [PyTschirper](https://github.com/christofmuc/PyTschirper) software here on github.

# Usage

As usual, you will need to import the pytschirp module into your python code before being able to use it:

    import pytschirp

The module itself is using native code, so on Windows you will need to point the sys.path to the folder where the file `pytschirp.cp36-win_amd64.pyd` is located. This is the name for a specific platform build (AMD64) for a specific python version (3.6). 

The pytschirp module provides three different main classes that can be used to manipulate synthesizers:

  1. A `Synthesizer` class representing the MIDI synth that you want to acesss. Note that this is a high level implementation of a specific synth like the Sequential DSI Prophet Rev 2, not a low level implementation where you have to deal with MIDI bytes, hexdumps, and sysex codes.
  2. A `Patch` class that represents one single patch (tone/program/...) of the synthesizer.
  3. A `PatchAttribute` class that represents one of the many parameters of a concrete Patch, and this PatchAttribute can be used to retrieve the value or modify a Patch. If the Patch was retrieved from a live device, the modification of the PatchAtribute at the same time changes the attribute on the live device (via MIDI).

## Synthesizer class

This is the first class that you will need to instantiate. For now, the only synth supported is the Prophet Rev2, so we can simply create an object that represents the synth:

    r = pytschirp.Rev2()

### Loading and saving sysex files

The synth class can be used to load and save sysex files in the native format for that synthesizer, e.g. to load the 512 factory patches of the Rev2 into a python array, just do (given the syx file is in the current working directory)

    factory_patches = r.loadSysex('Rev2_Programs_v1.0.syx')

You can also resave these patches into a new file, e.g. if you did some modification to them using the synthesizer object:

    r.saveSysex('modifed.syx', factory_patches)

This will produce a bank dump sysex file. To save only a single patch as an edit buffer dump (which will not overwrite any of the synth's storage places when sent to the synth):

    r.saveEditBuffer('editBuffer_dump_1.syx', factory_patches[12])

### Detecting the live device 

Until now, we were manipulating data structures of the Rev2 without needing any access to a physical device, but of course with the real thing, the fun only starts. If you have connected your Rev2 to your computer in a bidirectional MIDI connection (or just with USB), you can run

    r.detect()

which will take a few seconds depending on the number of MIDI ports on your computer, and a check can confirm that we are now talking to the synth:

    print(r.detected())  # Should give True

We can also check the MIDI interface/port that is used, in some scenarios this is important to know to debug:

    print(r.location())  # E.g. MIDI IN: Port 1 on MXPXT, MIDI OUT: Port 1 on MXPXT

### Doing live editing

There is a special type of patch that you retrieve from a successfully detected synthesizer object, and that is the current edit buffer. The edit buffer patch is different than patches retrieved e.g. via the loadSysex() command because any modification made to the editbuffer object will immediately cause MIDI messages to be generated and sent to the synth that make that modifcation also in the live device.

To retrieve the edit buffer patch, just do

    e = r.editBuffer()

## Patch class

To create an init patch for the Rev2, just create the object with

    p = pytschirp.Rev2Patch()

The patch itself is a collection of parameter values that determine the setup of the synthesizer for that particular sound. So the most important question to the patch is the list of parameters it has:

    print(p.parameterNames())

Don't be surpised if you get a list of more than one hundred parameters, the Rev2 is a pretty complex machine (154 to be precise, check with `len(p.parameterNames())`). 

### Attribute access

Once you know the name of an attribute you're interested in, you can get the attribute accessor object either via the attr() function, or just pretend the patch itself is a dictionary with the parameterNames as keys:

    cutoff_attribute = p.attr('Cutoff')
    cutoff_attribute = p['Cutoff']

### Patch name

The patch name currently is read only (that's on the list of improvements), and can be accessed via it's own attribute:

    print(p.name)

### Layers

As the Rev2 is actually dual layered, with both Layers having exactly the same parameters, you can retrieve the Layer A and Layer B via a method of the patch - and the returned object is in turn again a Patch, but all accessors to the parameters access layer A or B respectively:

    a = p.layer(0)
    b = p.layer(1)

## PatchAttribute class

The PatchAttribute class is your invisible helper in modifying the values of a patch. You will not need to instantiate any of these, or store objects of this type. They are used while interacting with the Patch class.

Most importantly, we support four different types of attributes:

    1. Single value
    2. Vector value
    3. Lookup value
    4. Vector of Lookup Values 

### Single Value

The single value attribute is the most basic one, and doesn't have any surprises. It feels like an Integer in python, and this is also how you can use it. The range is defined by the synth implementation. E.g. do

    cutoff_value = p["Cutoff"]  # gives the integer value of the cutoff    
    print(cutoff_value)  
    p["Cutoff"] = 128  # Allows direct setting of the integer value
    print(p["Cutoff"])  # Shows new value in patch

### Vector Value

This behaves like an integer vector in Python, and us used e.g. for the gated sequencer tracks, each track being a vector of length 16:

    print(p['Seq Track 1'])  # Produces e.g. [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    p['Seq Track 1'] = [1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3, 2, 1]  # Numbers
    print(p['Seq Track 1'])
    import random
    p['Seq Track 1'] = [random.randrange(30, 80) for _ in range(16)]  # Randomize
    print(p['Seq Track 1'])

### Lookup Value 

Many parameters act more like enums than as continuous values, we call those Lookup Values because there is effectively a lookup table or function to produce a descriptive string from the stored number. This is used automatically when printing a value:

    print(p['Gated Seq Mode'])  # Prints e.g. 'Normal'

You can retrieve the integer value again via the get() function 

    print(p['Gated Seq Mode'].get())  # Prints e.g. '0'

Setting currently doesn't support the reverse lookup, but needs to integer number:

    p['Gated Seq Mode'] = 2


### Vector of Lookup Values

Nothing special here, these can be treated exactly like the vector valued attributes, the only advantage is that they will produce a nicer display via the lookup table when printed. E.g. you get the note names displayed when you print out the note track from the poly sequencer:

    print(p['Poly Seq Note 1'])  # would give something like ['D#3', 'A#4', 'D#5', ...]
    print(p['Poly Seq Note 1'].get())  # would give something like [60, 64, 255, 255, 60, ...]

## Licensing

As some substantial work has gone into the development of this, I decided to offer a dual license - AGPL, see the LICENSE.md file for the details, for everybody interested in how this works and willing to spend some time her- or himself on this, and a commercial MIT license available from me on request. Thus I can help the OpenSource community without blocking possible commercial applications.

## Contributing

All pull requests and issues welcome, I will try to get back to you as soon as I can. Due to the dual licensing please be aware that I will need to request transfer of copyright on accepting a PR. 

## About the author

Christof is a lifelong software developer having worked in various industries, and can't stop his programming hobby anyway. 
