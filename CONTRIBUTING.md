# Contributing to the OpenFOAM-preCICE adapter

Hi there! Amazing that you want to contribute to this project!
Please have a look at the page [contributing to preCICE](https://precice.org/community-contribute-to-precice.html)
and watch out for more specific details in this file.

## Changelog

Instead of directly editing `CHANGELOG.md`, please add a file `123.md`
in `changelog-entries`, where `123` your pull request number. This helps reduce
merge conflicts and we will merge these files at the time we release a new version.

## Checking the Markdown files

Every contribution is automatically checked for issues in the `.md` files.
You can run these tests locally by installing [markdownlint-cli](https://github.com/igorshubovych/markdownlint-cli)
from npm:

```bash
npm install -g markdownlint-cli
```

If you get any permission errors when trying to install from npm, [set the npm prefix to a local directory](https://stackoverflow.com/a/54170648/2254346).

After installing, you can check all files in the current directory with:

```bash
markdownlint .
```

There are also extensions for many editors that automatically check Markdown files with the same system.

## Links in the Markdown files

Please use static links when refering to preCICE resources outside of this repository. While relative paths will also work in the rendered website, they will not work in other contexts.

We check these links automatically, but you can also check them locally with [markdown-link-check](https://github.com/tcort/markdown-link-check), which works similarly to markdownlint.
