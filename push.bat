git add --all
git reset -- emsdk
git commit -m "update"
git push https://github.com/beans42/maze-solve-web.git master

cd out
git init
git branch web
git add --all
git commit -m "update"
git push -f https://github.com/beans42/maze-solve-web.git web
rm -rf .git