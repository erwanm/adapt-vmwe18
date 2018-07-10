# ADAPT CENTRE participation to the shared task on Verbal Multiword Expressions 2017 (VMWE17)

*Authors: Erwan Moreau, Ashjan Alsulaimani and Alfredo Maldonado*

*Content coming soon (ETA: mid-July 2018)*

## Links

* [Shared task website](http://multiword.sourceforge.net/PHITE.php?sitesig=CONF&page=CONF_05_MWE_2017___lb__EACL__rb__)
* [Shared Task data (gitlab)](https://gitlab.com/parseme/sharedtask-data)
* Our paper will be published in August, coming soon.



## Description

*(This short description assumes that the reader is familiar with the task; if not, please see link above)*

This system attempts to exploit the dependency tree structure of the
sentences in order to identify MWEs. This is achieved by training a
tree-structured CRF model which takes into account conditional
dependencies between the nodes of the tree (node-parent and possibly
node-next sibling). The system is also trained to predict MWE
categories. The tree-structured CRF software used is [XCRF](http://treecrf.gforge.inria.fr/).


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
