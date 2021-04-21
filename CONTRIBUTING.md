# Contributing to the OpenFOAM-preCICE adapter

Hi there! Amazing that you want to contribute to this project!
Please have a look at the page [contributing to preCICE](https://precice.org/community-contribute-to-precice.html)
and watch out for more specific details in this file.

## Changelog

Instead of directly editing `CHANGELOG.md`, please add a file `123.md`
in `changelog-entries`, where `123` your pull request number. This helps reduce
merge conflicts and we will merge these files at the time we release a new version.

## Links in the Markdown files

Please use static links when refering to preCICE resources outside of this repository. While relative paths will also work in the rendered website, they will not work in other contexts.

## Checking the Markdown files

Every contribution is automatically checked for issues in the `.md` files.
You can run these tests locally by installing [act](https://github.com/nektos/act).

After installing, you can run our CI locally. Go to the root folder of this repository and run:

```bash
act -j check_md
```
