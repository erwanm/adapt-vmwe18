#usage: java xcrf.XcrfOperations train [-v] [-d attributeNameForInternalNode attributeNameForTextLeaf attributeNameForAttribute | -c annotationReaderPath] crfPath corpusPath resultPath
#
#   -v    prints log-likelihood at each maximization step
#   -c    use custom annotation reader
#   -d    use default annotation reader

java -Xms1g -Xmx1g -jar lib/xcrf.jar train $*
