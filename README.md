# ADAPT CENTRE participation to the shared task on Verbal Multiword Expressions 2017 (VMWE17)

*Authors: Erwan Moreau, Ashjan Alsulaimani and Alfredo Maldonado*

*Content coming soon (ETA: mid-July 2018)*

## Links

* [Shared task website](http://multiword.sourceforge.net/PHITE.php?sitesig=CONF&page=CONF_05_MWE_2017___lb__EACL__rb__)
* [Shared Task data (gitlab)](https://gitlab.com/parseme/sharedtask-data)
* Our paper will be published in August, coming soon.


## Requirements

* [libxm2](http://www.xmlsoft.org/) must be installed to compile the *dep-tree* system, including the source libraries (header files)
  * on Ubuntu the most convenient way is to install the package `libxml2-dev`: `sudo apt install libxml2-dev`.
* [CRF++](https://taku910.github.io/crfpp/) must be installed and accessible via `PATH`
* [Wapiti](https://wapiti.limsi.fr/) must be installed and accessible via `PATH`
* The shared task data can be downloaded or cloned from https://gitlab.com/parseme/sharedtask-data

  
  
## Installation

From the main directory run:

```
source setup-path.sh
```

This will compile the code if needed and add the relevant directories to `PATH`. You can add this to your `.bashrc` file in order to have the `PATH` set up whenever you open a new session.

## Usage
