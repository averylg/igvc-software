//
// Created by aaronmao on 12/27/19.
//

#ifndef SRC_FAKECONE_LAYER_H
#define SRC_FAKECONE_LAYER_H


#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include "gridmap_layer.h"
#include "fakecone_layer_config.h"

namespace fakecone_layer {
    class FakeconeLayer : public gridmap_layer::GridmapLayer {
    public:
        using PointCloud = pcl::PointCloud<pcl::PointXYZ>;

        FakeconeLayer();

        void onInitialize() override;

        void updateCosts(costmap_2d::Costmap2D &master_grid, int min_i, int min_j, int max_i, int max_j) override;

        void updateBounds(double robot_x, double robot_y, double robot_yaw, double *min_x, double *min_y, double *max_x,
                          double *max_y) override;

    private:
        static constexpr auto logodds_layer = "logodds";
        static constexpr auto probability_layer = "probability";

        ros::NodeHandle nh_;
        ros::NodeHandle private_nh_;
        grid_map::Matrix *layer_{};
        //TODO: solve constructor issue with provided parameter
        FakeconeLayerConfig config_;
        cv::Mat line_buffer_;
        cv::Mat freespace_buffer_;
        cv::Mat not_lines_;

        //TODO: figure out what is Eigen:Vecotor3d - cached_rays_
        ros::Publisher gridmap_pub_;
        ros::Publisher costmap_pub_;
        std::vector<int8_t> cost_translation_table_;

        costmap_2d::Costmap2D costmap_2d_{};

        void initGridmap();
        void initPubSub();

        void matchCostmapDims(const costmap_2d::Costmap2D& master_grid);

        void updateProbabilityLayer();
        void transferToCostmap();
        void updateRollingWindow();
        void updateStaticWindow();
        void debugPublishMap();
        void publishCostmap();
        void initCostTranslationTable();

        //TODO: Replace with own implementation
        void markHit(const grid_map::Index, double distance);

        void markEmpty(const grid_map::Index, double distance);
    };
}

#endif //SRC_FAKECONE_LAYER_H
