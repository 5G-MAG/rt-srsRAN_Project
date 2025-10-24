<h1 align="center">NG-RAN (with support for 5G Multicast Broadcast Services - 5MBS)</h1>
<p align="center">
  <img src="https://img.shields.io/badge/Status-Under_Development-yellow" alt="Under Development">
  <img src="https://img.shields.io/badge/License-AGPL_v3-blue.svg" alt="License">
</p>

## Introduction

This branch hosts MBS-related NG-RAN procedures. In particular, it contains a modified version of srsRAN_Project which includes the related functions to establish a Broadcast MBS session at the CU (control unit).
The actual implementation handles the NGAP messaging from the 5G core to create and register the requested session from the core. At the moment only the CU MBS funcionality is present.
The implemented functionality corresponds to the signaling described in the document [38.401](https://www.etsi.org/deliver/etsi_ts/138400_138499/138401/17.08.00_60/ts_138401v170800p.pdf) v17.8.0 section 8.15, the steps 1, 2, 3, 7, 8, and 10.

Additional information can be found at: https://5g-mag.github.io/Getting-Started/pages/5g-multicast-broadcast-services/

### About the implementation

This implementation is based on the [srsRAN_Project](https://github.com/srsran/srsRAN_Project).
For more information, please refer to the [main branch](https://github.com/5G-MAG/rt-srsRAN_Project).

## Downloading
```
git clone --recurse-submodules -b 5mbs https://github.com/5G-MAG/rt-srsRAN_Project.git ~/rt-srsRAN_Project_mbs
```

## Building, Installing and Running

Please follow the regular build, installation and running instructions from srsRAN_Project.

## Acknowledgements

The reference implementation of the MBS features was funded by the European Union through the [6G-SANDBOX](https://6g-sandbox.eu/) project.
