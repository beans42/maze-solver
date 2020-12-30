git add --all
git reset -- emsdk
git commit -m "update"
git push https://github.com/beans42/maze-solver.git web

pushd out
git init
git checkout -b gh-pages
git add --all
git commit -m "update"
git push -f https://github.com/beans42/maze-solver.git gh-pages
rm -rf .git
popd