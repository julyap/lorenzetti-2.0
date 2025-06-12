from setuptools import setup, find_packages

setup(
    name="gaugikernel",
    version="0.1.0",
    package_dir={'': 'src'},
    packages=find_packages(where='src'),
    install_requires=[
        'numpy',
        'rich_argparse',
    ],
    python_requires='>=3.7',
    extras_require={
        'dev': [
            'pytest>=6.0',
            'pytest-cov>=2.0',
            'flake8>=3.9',
            'black>=21.0',
        ],
        'docs': [
            'sphinx>=4.0',
            'sphinx-rtd-theme>=0.5',
        ],
    },
)
