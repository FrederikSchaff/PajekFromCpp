# PajekFromCpp
Stand alone c++ source code to create pajek .net and .paj
(including timeline feature for PajekToScgAnim) files from any simulation.

This code is generated and tested for the use with LSD by Marco Valente
(http://www.labsimdev.org/Joomla_1-3/, https://github.com/marcov64/Lsd),
a c++ based framework for agent-based modeling. It is not a substitute to the
network functionality implemented by Marcelo Pereira in current snapshots of
LSD. It is only ment to be used to export network-data from a simulation in such
a manner that it may be analysed and visualised with pajek or other software
that can make use of the .paj or .net format.  The intention is to provide full
flexibility and all the options that pajek by Andrej Mrvar
(http://mrvar.fdv.uni-lj.si/pajek/)
.paj (and .net) format provides. In addition, the .paj files created may be used
with PajekToScgAnim by Darko Brvar:
- PajekToSvgAnim (by Darko Brvar): http://mrvar.fdv.uni-lj.si/pajek/PajekToSvgAnim/PajekToSvgAnim.zip
- test datasets: http://mrvar.fdv.uni-lj.si/pajek/PajekToSvgAnim/AnimData.zip
- manual: http://freeweb.siol.net/dbrvar2/PajekToSvgAnim11081.pdf


*Usage*:
Include the pajek.cpp, which will in turn include the header file.
For more information see pajek.h file.

The code is provided "as is".

For further information on Pajek see also:

Mrvar, Andrej; Batagelj, Vladimir (2016): Analysis and visualization of large
networks with program package Pajek. In: Complex Adaptive Systems Modeling 4(1),
 S. 1–8. DOI: 10.1186/s40294-016-0017-8.

Nooy, Wouter de; Mrvar, Andrej; Batagelj, Vladimir (2011): Exploratory social
network analysis with Pajek. Rev. and expanded 2nd ed. England, New York:
Cambridge University Press (Structural analysis in the social sciences, 34).
