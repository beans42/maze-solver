git add --all
git reset -- emsdk
git commit -m "update"
git push https://github.com/beans42/web-maze.git web

pushd out
git init
git checkout -b web
git add --all
git commit -m "update"
git push -f https://github.com/beans42/web-maze.git gh-pages
rm -rf .git
popd