# ADAPT CENTRE participation to the shared task on Verbal Multiword Expressions 2017 (VMWE17)

*Authors: Erwan Moreau, Ashjan Alsulaimani and Alfredo Maldonado*

*Content coming soon (ETA: mid-July 2018)*

## Links

* [Shared task website](http://multiword.sourceforge.net/PHITE.php?sitesig=CONF&page=CONF_05_MWE_2017___lb__EACL__rb__)
* [Shared Task data (gitlab)](https://gitlab.com/parseme/sharedtask-data)
* Our paper will be published in August, link coming soon (if I don't forget!).



## Description

This repository contains two distinct systems for detecting verbal multiword expressions from text. This short description assumes that the reader is familiar with the task; if not, please see link above.

### Subdirectory dep-tree (a.k.a CRF-DepTree-categs)


This system attempts to exploit the dependency tree structure of the
sentences in order to identify MWEs. This is achieved by training a
tree-structured CRF model which takes into account conditional
dependencies between the nodes of the tree (node-parent and possibly
node-next sibling). The system is also trained to predict MWE
categories. The tree-structured CRF software used is [XCRF](http://treecrf.gforge.inria.fr/).

### Subdirectory seq (a.k.a CRF-Seq-nocategs)

A robust sequential method which can work with only lemmas and morphosyntactic tags. It uses the [Wapiti](https://wapiti.limsi.fr/) CRF sequence labeling software.

## Requirements

* [libxml2](http://www.xmlsoft.org/) must be installed to compile the *dep-tree* system, including the source libraries (header files)
  * on Ubuntu the most convenient way is to install the package `libxml2-dev`: `sudo apt install libxml2-dev`.
* [CRF++](https://taku910.github.io/crfpp/) must be installed and accessible via `PATH`
* [Wapiti](https://wapiti.limsi.fr/) must be installed and accessible via `PATH`
* The shared task data can be downloaded or cloned from https://gitlab.com/parseme/sharedtask-data
* [XCRF](http://treecrf.gforge.inria.fr/) is also required but provided in this repository

  
  
## Installation

From the main directory run:

```
source setup-path.sh
```

This will compile the code if needed and add the relevant directories to `PATH`. You can add this to your `.bashrc` file in order to have the `PATH` set up whenever you open a new session.

## Usage

### Dependency tree approach

#### Training

From the directory `dep-tree`:

```
train-test-class-method.sh -l sharedtask-data/1.1/FR/train.cupt conf/minimal.conf model-dir output-dir
```

* To run the script from a different directory, one has to provide the path to the XCRF directory in the following way:
```
train-test-class-method.sh -o '-x dep-tree/xcrf-1.0/' -l sharedtask-data/1.1/FR/train.cupt dep-tree/conf/minimal.conf model-dir output-dir
```

#### Testing

* See `-o` above to run the script from a different directory.

#### Multiple config files and datasets

### Sequential approach

## Details

### Configuration files

The scripts are meant to be used with configuration files which contain values for the parameters. Examples can be found in the directory `conf`. Additionally, a batch of configuration files can be generated using e.g.:

```
# caution: generates a set of config files (written to directory 'configs')
mkdir configs; echo dep-tree/conf/large.multi-conf | expand-multi-config.pl configs
```

In order to generate a different set of configurations, either customize the values that a parameter can take in `conf/options.multi-conf` or use the `-r` option to generate a random subset of config files, e.g.:

```
# generate a random 50 config files
mkdir configs; echo dep-tree/conf/large.multi-conf | expand-multi-config.pl -r 50 configs
```




