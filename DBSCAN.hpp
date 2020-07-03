#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/octree/octree.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/visualization/pcl_visualizer.h>

#include <iostream>
#include <vector>
using namespace pcl;
using namespace std;

class DBSCAN {
 public:
  DBSCAN(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud, double eps, double MinPts)
      : eps(eps), MinPts(MinPts) {
    cloud_ =
        pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>);
    cloud_ = cloud;
    origin_cloud_ =
        pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>);
    origin_cloud_ = cloud;
    cluster_type = vector<int>(cloud->points.size());
    neighbourPoints.resize(cloud->points.size());
    neighbourDistance.resize(cloud->points.size());
  }
  double MinPts;
  double eps;
  ~DBSCAN() {}
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_;
  pcl::PointCloud<pcl::PointXYZ>::Ptr origin_cloud_;
  vector<pcl::PointCloud<pcl::PointXYZ>::Ptr> result_cloud_;
  vector<int> cluster_type;
  vector<int> core_points;
  vector<int> result_points;
  int method_ = 0;
  const int CORE_POINT = 0;
  const int BOARD_POINT = 1;
  const int NOISE_POINT = 2;
  const int KD_TREE = 0;
  const int OCT_TREE = 1;
  std::vector<std::vector<int>> neighbourPoints;
  std::vector<std::vector<float>> neighbourDistance;
  void start_scan();
  void select_kernel();
  void find_independent();
};