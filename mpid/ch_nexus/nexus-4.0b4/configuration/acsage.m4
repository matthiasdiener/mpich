AC_DEFUN(LAC_SAGE_PATH,
[for sage_root in $srcdir/../sage2 $srcdir/../../sage2; do
   if test -d $sage_root; then
      lac_save_dir=`pwd`
      cd $sage_root
      sage_root=`pwd`	
      cd $lac_save_dir	
      AC_SUBST(sage_root)
      break
   fi
 done
for sage_build in sage2 ../sage2 ../../sage2; do
   if test -d $sage_build; then
      AC_SUBST(sage_build)
      break
   fi
done
])


