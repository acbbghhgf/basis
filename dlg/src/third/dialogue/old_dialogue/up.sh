#!/bin/bash
#update for externals
svn propset svn:externals . -F svn.externals
#remove old version
#rm -rf wvite
svn up .
