#include "DBSCAN.hpp"

void viewerOneOff(pcl::visualization::PCLVisualizer &viewer, double x, double y,
                  double z, string name) {
  pcl::PointXYZ o;
  o.x = x;
  o.y = y;
  o.z = z;
  viewer.addSphere(o, 0.001, 255, 0, 0, name, 0);
}

void showCloud(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud1,
               pcl::PointCloud<pcl::PointXYZ>::Ptr cloud2) {
  boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(
      new pcl::visualization::PCLVisualizer("3D Viewer"));
  int v1(0);
  viewer->createViewPort(0.0, 0.0, 0.5, 1.0, v1);
  viewer->setBackgroundColor(0, 0, 0, v1);
  viewer->addPointCloud<pcl::PointXYZRGB>(cloud1, "sample cloud1", v1);

  int v2(0);
  viewer->createViewPort(0.5, 0.0, 1.0, 1.0, v2);
  viewer->addPointCloud<pcl::PointXYZ>(cloud2, "sample cloud2", v2);
  viewer->setBackgroundColor(0.3, 0.3, 0.3, v2);
  // viewer->addCoordinateSystem(1.0);

  viewer->initCameraParameters();
  while (!viewer->wasStopped()) {
    viewer->spinOnce(100);
    // boost::this_thread::sleep(boost::posix_time::microseconds(100000));
  };
}
void DBSCAN::start_scan() {
  start = clock();
  select_kernel();
  find_independent();
}
void DBSCAN::select_kernel() {
  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
  float resolution = 0.0001f;
  pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> octree(resolution);
  if (method_ == KD_TREE) {
    kdtree.setInputCloud(cloud_);
  } else if (method_ == OCT_TREE) {
    octree.setInputCloud(cloud_);
    octree.addPointsFromInputCloud();
  }
  vector<int> visit;
  for (auto i = 0; i < cloud_->points.size(); i++) {
    if (neighbourDistance[i].size()) continue;
    if (method_ == KD_TREE) {
      // kdtree.radiusSearch(cloud_->points[i], eps, neighbourPoints[i],
      //                     neighbourDistance[i]);
      kdtree.nearestKSearch(cloud_->points[i], 30, neighbourPoints[i],
                            neighbourDistance[i]);
    } else if ((method_ == OCT_TREE)) {
      octree.radiusSearch(cloud_->points[i], eps, neighbourPoints[i],
                          neighbourDistance[i]);
    }
    float max = sqrt(
        *max_element(neighbourDistance[i].begin(), neighbourDistance[i].end()));
    cout << max << " num " << MinPts << " " << neighbourDistance.size()
         << neighbourDistance[i][1] << endl;
    for (auto j = 0; j < neighbourPoints[i].size() && max < eps; j++) {
      kdtree.radiusSearch(cloud_->points[neighbourPoints[i][j]], max,
                          neighbourPoints[neighbourPoints[i][j]],
                          neighbourDistance[neighbourPoints[i][j]]);
      cout << "neighbour" << neighbourPoints[neighbourPoints[i][j]].size()
           << endl;
      if (neighbourPoints[neighbourPoints[i][j]].size() >= MinPts) {
        cluster_type.push_back(CORE_POINT);
        core_points.push_back(neighbourPoints[i][j]);
        //   cout << "core cluster" << neighbourPoints[i].size() << endl;
      } else if (neighbourPoints[neighbourPoints[i][j]].size() > MinbPts) {
        cluster_type.push_back(BOUND_POINT);
        bound_points.push_back(neighbourPoints[i][j]);
      } else {
        cluster_type.push_back(NOISE_POINT);
      }
    }
  }
  cout << "core_points" << core_points.size() << endl;
  cout << "bound_points" << bound_points.size() << endl;
}
vector<int> DBSCAN::vectors_intersection(vector<int> v1, vector<int> v2) {
  vector<int> v;
  sort(v1.begin(), v1.end());
  sort(v2.begin(), v2.end());
  set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
                   back_inserter(v));  //求交集
  return v;
}

//两个vector求并集
vector<int> DBSCAN::vectors_set_union(vector<int> v1, vector<int> v2) {
  vector<int> v;
  sort(v1.begin(), v1.end());
  sort(v2.begin(), v2.end());
  set_union(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(v));
  return v;
}
vector<int> DBSCAN::vectors_set_diff(vector<int> v1, vector<int> v2) {
  vector<int> v;
  sort(v1.begin(), v1.end());
  sort(v2.begin(), v2.end());
  set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(v));
  return v;
}
void DBSCAN::find_independent() {
  cout << "start find intersection of cluster and union them" << endl;
  for (int i = 0; i < core_points.size(); i++) {
    if (neighbourPoints[core_points[i]].size() == 0) continue;
    for (int j = 0; j < core_points.size(); j++) {
      if (j == i || neighbourPoints[core_points[j]].size() == 0) continue;
      vector<int> result;
      result = vectors_intersection(neighbourPoints[core_points[i]],
                                    neighbourPoints[core_points[j]]);
      if (result.size() > 20) {
        if (neighbourPoints[core_points[i]].size() < 500 &&
            neighbourPoints[core_points[j]].size() < 500 &&
            (neighbourPoints[core_points[i]].size() +
             neighbourPoints[core_points[j]].size()) < 600) {
          neighbourPoints[core_points[i]] = vectors_set_union(
              neighbourPoints[core_points[i]], neighbourPoints[core_points[j]]);
          neighbourPoints[core_points[j]].clear();
        }
        // else {
        //   neighbourPoints[core_points[j]] = vectors_set_diff(
        //       neighbourPoints[core_points[i]],
        //       neighbourPoints[core_points[j]]);
        //   // neighbourPoints[core_points[j]].clear();
        // }
      }
    }
  }
  for (int i = 0; i < bound_points.size() && use_edge; i++) {
    int max_intersect = 0;
    int max_index = -1;
    for (int j = 0; j < core_points.size(); j++) {
      if (neighbourPoints[core_points[j]].size() == 0) continue;
      vector<int> result;
      result = vectors_intersection(neighbourPoints[bound_points[i]],
                                    neighbourPoints[core_points[j]]);
      if (result.size() > max_intersect) {
        max_intersect = result.size();
        max_index = core_points[j];
      }
    }
    if (max_index != -1) {
      neighbourPoints[max_index] = vectors_set_union(
          neighbourPoints[max_index], neighbourPoints[bound_points[i]]);
    }
  }
  vector<int> final_cluster;

  pcl::PointCloud<pcl::PointXYZRGB>::Ptr result_cloud(
      new pcl::PointCloud<pcl::PointXYZRGB>);
  this->cluster_center =
      pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>);
  for (auto i = 0; i < core_points.size(); i++) {
    if (neighbourPoints[core_points[i]].size() == 0 ||
        neighbourPoints[core_points[i]].size() < 200)
      continue;
    cout << "find_cluster" << neighbourPoints[core_points[i]].size() << endl;
    auto iter_1 = cloud_->begin() + i;
    auto iter_2 = neighbourPoints.begin() + i;
    auto iter_3 = neighbourDistance.begin() + i;
    int R = rand() % 255;
    int G = rand() % 255;
    int B = rand() % 255;
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_cluster(
        new pcl::PointCloud<pcl::PointXYZ>);

    for (auto j = 0; j < neighbourPoints[core_points[i]].size(); j++) {
      pcl::PointXYZRGB point;
      point.r = R;
      point.g = G;
      point.b = B;
      point.x = cloud_->points[neighbourPoints[core_points[i]][j]].x;
      point.y = cloud_->points[neighbourPoints[core_points[i]][j]].y;
      point.z = cloud_->points[neighbourPoints[core_points[i]][j]].z;
      result_cloud->points.push_back(point);
      cloud_cluster->points.push_back(
          cloud_->points[neighbourPoints[core_points[i]][j]]);
    }
    cloud_cluster->width = cloud_cluster->points.size();
    cloud_cluster->height = 1;
    cloud_cluster->is_dense = true;
    sum_points += cloud_cluster->points.size();
    points_num.push_back(cloud_cluster->points.size());
    Eigen::Vector4f centroid;
    pcl::compute3DCentroid(*cloud_cluster, centroid);
    // std::cout << "The XYZ coordinates of the centroid are: (" <<
    // centroid[0]
    //           << ", " << centroid[1] << ", " << centroid[2] << ")."
    //           << std::endl;
    cluster_center->points.push_back(
        pcl::PointXYZ(centroid[0], centroid[1], centroid[2]));
    cluster_centroid.push_back(centroid);
    this->result_cloud_.push_back(cloud_cluster);
  }
  cluster_center->width = cluster_center->points.size();
  cluster_center->height = 1;
  cluster_center->is_dense = true;
  cout << "the sum of clusters: " << this->result_cloud_.size() << endl;
  result_cloud->width = result_cloud->points.size();
  result_cloud->height = 1;
  result_cloud->is_dense = true;

  end = clock();
  double endtime = (double)(end - start) / CLOCKS_PER_SEC;
  cout << "Total time:" << end << "  " << start << "  " << endtime << "s"
       << endl;  // s为单位
  vector<pcl::PointXYZ> cluster_vectors;
  vector<pcl::PointXYZ> centroidXYZ_value;
  if (cluster_center->points.size() > 1) {
    pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
    kdtree.setInputCloud(cluster_center);

    for (int i = 0; i < cluster_center->points.size(); i++) {
      cluster temp;
      temp.index = i;
      cluster_score.push_back(temp);
      vector<int> indices;
      vector<float> dists;
      kdtree.nearestKSearch(cluster_center->points[i],
                            int(cluster_center->points.size() / 2) + 1, indices,
                            dists);
      double sum = 0;
      for (auto dist : dists) {
        sum = sum + dist;
      }
      cluster_score[i].dense = sum;
      // pca(result_cloud_[i]);
      // Eigen::Vector4f centroid;
      // Eigen::Matrix3f covariance_matrix;

      // // Extract the eigenvalues and eigenvectors
      // Eigen::Vector3f eigen_values;
      // Eigen::Matrix3f eigen_vectors;

      // // pcl::compute3DCentroid(*result_cloud_[i], centroid);

      // // Compute the 3x3 covariance matrix
      // pcl::computeCovarianceMatrix(*result_cloud_[i], cluster_centroid[i],
      //                              covariance_matrix);
      // pcl::eigen33(covariance_matrix, eigen_vectors, eigen_values);
      // PointXYZ centroidXYZ;
      // centroidXYZ.getVector4fMap() = cluster_centroid[i];
      // centroidXYZ_value.push_back(centroidXYZ);
      // PointXYZ Point1 =
      //     PointXYZ((cluster_centroid[i](0) + eigen_vectors.col(0)(0)),
      //              (cluster_centroid[i](1) + eigen_vectors.col(0)(1)),
      //              (cluster_centroid[i](2) + eigen_vectors.col(0)(2)));
      // cluster_vectors.push_back(Point1);

      // _viewer->addLine<pcl::PointXYZRGB> (centroid, eigen_vectors, "line");
    }
    // sort(cluster_score.begin(), cluster_score.end(), cmp);
  } else {
    cluster temp;
    temp.index = 0;
    cluster_score.push_back(temp);
  }
  // for (int i = 0; i < cluster_score.size(); i++) {
  //   cluster_score[i].dense_index = i + 1;
  //   // cout << cluster_score[i].dense_index << "  " <<
  //   cluster_score[i].dense
  //   //      << " " << cluster_score[i].index << endl;
  // }
  if (view_on) {
    boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(
        new pcl::visualization::PCLVisualizer("3D Viewer"));
    int v1(0);
    viewer->createViewPort(0.0, 0.0, 0.5, 1.0, v1);
    viewer->setBackgroundColor(0, 0, 0, v1);
    viewer->addPointCloud<pcl::PointXYZRGB>(result_cloud, "sample cloud1", v1);

    int v2(0);
    viewer->createViewPort(0.5, 0.0, 1.0, 1.0, v2);
    viewer->addPointCloud<pcl::PointXYZ>(origin_cloud_, "sample cloud2", v2);
    viewer->setBackgroundColor(0.3, 0.3, 0.3, v2);
    // viewer->addCoordinateSystem(1.0);

    viewer->initCameraParameters();
    for (int i = 0; i < cluster_centroid.size(); i++) {
      std::stringstream ss;
      ss << "cloud_cluster_" << i;
      viewerOneOff(*viewer, cluster_centroid[i][0], cluster_centroid[i][1],
                   cluster_centroid[i][2], ss.str());
      showCloud(result_cloud, result_cloud_[i]);
      // std::stringstream ss2;
      // ss2 << "arrow" << i;
      // viewer->addArrow(cluster_vectors.at(i), centroidXYZ_value.at(i), 0.5,
      // 0.5,
      //                  0.5, false, ss2.str());
    }
    while (!viewer->wasStopped()) {
      viewer->spinOnce(100);
      // boost::this_thread::sleep(boost::posix_time::microseconds(100000));
    };
  }
}
int main() {
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

  pcl::io::loadPCDFile(
      "/home/gjx/orbslam/catkin_ws/src/ZJUBinPicking/pcd_files/filter_5.pcd",
      *cloud);
  double dense = 0.01;
  double result_dense;
  double min = 10000000;
  // cout << dense << endl;
  // for (int i = 0; i < 5; i++) {
  //   DBSCAN gather(cloud, dense, 40);  // 0.011
  //   gather.view_on = 0;
  //   gather.start_scan();

  //   double avr = gather.sum_points / gather.points_num.size();
  //   double var = 0;
  //   for (int i = 0; i < gather.points_num.size(); i++) {
  //     var += (gather.points_num[i] - avr) * (gather.points_num[i] - avr);
  //   }
  //   var = var / gather.points_num.size();
  //   cout << "var" << var << endl;

  //   if (var < min) {
  //     min = var;
  //     result_dense = dense;
  //   }
  //   dense -= 0.0001;
  // }
  cout << "result dense" << result_dense << endl;
  DBSCAN gather(cloud, 0.01, 30);  // 0.011
  gather.start_scan();
}