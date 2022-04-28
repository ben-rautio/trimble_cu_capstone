from setuptools import setup
#https://answers.ros.org/question/367793/including-a-python-module-in-a-ros2-package/
package_name = 'trimble_ipd'
submodules = 'trimble_ipd/pySerialTransfer'
pnp_path = 'trimble_ipd/PNP'
gstreamer_path = 'trimble_ipd/gstreamer_pipeline'
setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name, 
                submodules,
                gstreamer_path,
                pnp_path,
    ],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('trimble_ipd/pnp_modular.py'),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='ben',
    maintainer_email='ben.e.rautio@gmail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'ipd_rawimg_pub = trimble_ipd.ipd_rawimg_pub:main',
            'ipd_pose_estimator = trimble_ipd.ipd_pose_estimator:main',
            'demo_listener = trimble_ipd.demo_listener:main'
        ],
    },
)
