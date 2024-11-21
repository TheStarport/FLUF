# FLUF

## Docs

```
pip install -r requirements.txt
doxygen Doxyfile
cd docs
sphinx-build -M html . ./_build
python -m http.server 5000 -d ./build/html
```