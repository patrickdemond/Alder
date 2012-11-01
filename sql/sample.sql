-- MySQL dump 10.13  Distrib 5.5.24, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: alder
-- ------------------------------------------------------
-- Server version	5.5.24-0ubuntu0.12.04.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `Cineloop`
--

DROP TABLE IF EXISTS `Cineloop`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Cineloop` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `update_timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `create_timestamp` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `exam_id` int(10) unsigned NOT NULL,
  `number` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_cineloop_exam_id` (`exam_id`),
  CONSTRAINT `fk_cineloop_exam_id` FOREIGN KEY (`exam_id`) REFERENCES `Exam` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=19 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Cineloop`
--

LOCK TABLES `Cineloop` WRITE;
/*!40000 ALTER TABLE `Cineloop` DISABLE KEYS */;
INSERT INTO `Cineloop` VALUES (1,'2012-11-01 16:22:16','0000-00-00 00:00:00',1,1),(2,'2012-11-01 16:22:16','0000-00-00 00:00:00',1,2),(3,'2012-11-01 16:22:16','0000-00-00 00:00:00',1,3),(4,'2012-11-01 16:22:16','0000-00-00 00:00:00',2,1),(5,'2012-11-01 16:22:16','0000-00-00 00:00:00',2,2),(6,'2012-11-01 16:22:16','0000-00-00 00:00:00',2,3),(7,'2012-11-01 16:22:16','0000-00-00 00:00:00',3,1),(8,'2012-11-01 16:22:16','0000-00-00 00:00:00',3,2),(9,'2012-11-01 16:22:16','0000-00-00 00:00:00',3,3),(10,'2012-11-01 16:22:16','0000-00-00 00:00:00',4,1),(11,'2012-11-01 16:22:16','0000-00-00 00:00:00',4,2),(12,'2012-11-01 16:22:16','0000-00-00 00:00:00',4,3),(13,'2012-11-01 16:22:16','0000-00-00 00:00:00',5,1),(14,'2012-11-01 16:22:16','0000-00-00 00:00:00',5,2),(15,'2012-11-01 16:22:16','0000-00-00 00:00:00',5,3),(16,'2012-11-01 16:22:16','0000-00-00 00:00:00',6,1),(17,'2012-11-01 16:22:16','0000-00-00 00:00:00',6,2),(18,'2012-11-01 16:22:16','0000-00-00 00:00:00',6,3);
/*!40000 ALTER TABLE `Cineloop` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Image`
--

DROP TABLE IF EXISTS `Image`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Image` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `update_timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `create_timestamp` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `exam_id` int(10) unsigned NOT NULL,
  `cineloop_id` int(10) unsigned NOT NULL,
  `frame` int(11) NOT NULL,
  `interviewer_defined` tinyint(1) NOT NULL,
  `min` float DEFAULT NULL,
  `max` float DEFAULT NULL,
  `mean` float DEFAULT NULL,
  `sd` float DEFAULT NULL,
  `n` int(11) DEFAULT NULL,
  `file` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_image_exam_id` (`exam_id`),
  KEY `fk_image_cineloop_id` (`cineloop_id`),
  KEY `dk_cineloop_id_frame` (`cineloop_id`,`frame`),
  CONSTRAINT `fk_image_exam_id` FOREIGN KEY (`exam_id`) REFERENCES `Exam` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `fk_image_cineloop_id` FOREIGN KEY (`cineloop_id`) REFERENCES `Cineloop` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Image`
--

LOCK TABLES `Image` WRITE;
/*!40000 ALTER TABLE `Image` DISABLE KEYS */;
INSERT INTO `Image` VALUES (1,'2012-11-01 16:37:46','0000-00-00 00:00:00',1,1,1,1,NULL,NULL,NULL,NULL,NULL,''),(2,'2012-11-01 16:37:55','0000-00-00 00:00:00',2,4,1,1,NULL,NULL,NULL,NULL,NULL,''),(3,'2012-11-01 16:38:01','0000-00-00 00:00:00',3,7,1,1,NULL,NULL,NULL,NULL,NULL,''),(4,'2012-11-01 16:38:08','0000-00-00 00:00:00',4,10,1,1,NULL,NULL,NULL,NULL,NULL,''),(5,'2012-11-01 16:38:14','0000-00-00 00:00:00',5,13,1,1,NULL,NULL,NULL,NULL,NULL,''),(6,'2012-11-01 16:38:21','0000-00-00 00:00:00',6,16,1,1,NULL,NULL,NULL,NULL,NULL,'');
/*!40000 ALTER TABLE `Image` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Rating`
--

DROP TABLE IF EXISTS `Rating`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Rating` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `update_timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `create_timestamp` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `image_id` int(10) unsigned NOT NULL,
  `user_id` int(10) unsigned NOT NULL,
  `rating` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_rating_image_id` (`image_id`),
  KEY `fk_rating_user_id` (`user_id`),
  KEY `dk_rating` (`rating`),
  CONSTRAINT `fk_rating_image_id` FOREIGN KEY (`image_id`) REFERENCES `Image` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `fk_rating_user_id` FOREIGN KEY (`user_id`) REFERENCES `User` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Rating`
--

LOCK TABLES `Rating` WRITE;
/*!40000 ALTER TABLE `Rating` DISABLE KEYS */;
/*!40000 ALTER TABLE `Rating` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Exam`
--

DROP TABLE IF EXISTS `Exam`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Exam` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `update_timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `create_timestamp` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `study_id` int(10) unsigned NOT NULL,
  `laterality` enum('right','left') NOT NULL,
  `type` enum('cimt','plaque') NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_exam_study_id` (`study_id`),
  KEY `dk_laterality` (`laterality`),
  KEY `dk_type` (`type`),
  CONSTRAINT `fk_exam_study_id` FOREIGN KEY (`study_id`) REFERENCES `Study` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Exam`
--

LOCK TABLES `Exam` WRITE;
/*!40000 ALTER TABLE `Exam` DISABLE KEYS */;
INSERT INTO `Exam` VALUES (1,'2012-11-01 15:39:51','0000-00-00 00:00:00',1,'left','cimt'),(2,'2012-11-01 15:39:54','0000-00-00 00:00:00',1,'right','cimt'),(3,'2012-11-01 15:39:58','0000-00-00 00:00:00',2,'left','cimt'),(4,'2012-11-01 15:40:01','0000-00-00 00:00:00',2,'right','cimt'),(5,'2012-11-01 15:40:05','0000-00-00 00:00:00',3,'left','cimt'),(6,'2012-11-01 15:40:08','0000-00-00 00:00:00',3,'right','cimt');
/*!40000 ALTER TABLE `Exam` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Study`
--

DROP TABLE IF EXISTS `Study`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Study` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `update_timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `create_timestamp` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `uid` varchar(45) NOT NULL,
  `site` varchar(45) NOT NULL,
  `interviewer` varchar(45) NOT NULL,
  `datetime_acquired` datetime NOT NULL,
  `datetime_reviewed` datetime DEFAULT NULL,
  `note` text,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_uid` (`uid`),
  KEY `dk_uid` (`uid`),
  KEY `dk_datetime_reviewed` (`datetime_reviewed`),
  KEY `dk_site` (`site`),
  KEY `dk_datetime_acquired` (`datetime_acquired`),
  KEY `dk_interviewer` (`interviewer`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Study`
--

LOCK TABLES `Study` WRITE;
/*!40000 ALTER TABLE `Study` DISABLE KEYS */;
INSERT INTO `Study` VALUES (1,'2012-11-01 14:37:24','0000-00-00 00:00:00','B547411','Site #1','Edward','2012-06-01 12:00:00',NULL,NULL),(2,'2012-11-01 14:39:06','0000-00-00 00:00:00','B800183','Site #1','Tubbs','2012-06-02 14:00:00',NULL,NULL),(3,'2012-11-01 14:39:41','0000-00-00 00:00:00','J983695','Site #2','Lazarou','2012-07-15 09:00:00',NULL,NULL);
/*!40000 ALTER TABLE `Study` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `User`
--

DROP TABLE IF EXISTS `User`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `User` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `update_timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `create_timestamp` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `name` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `last_login` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_name` (`name`),
  KEY `dk_last_login` (`last_login`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `User`
--

LOCK TABLES `User` WRITE;
/*!40000 ALTER TABLE `User` DISABLE KEYS */;
INSERT INTO `User` VALUES (1,'2012-11-01 14:24:27','2012-11-01 14:24:27','administrator','yQ7sfVQNBaEWRA/M1+/rtNp8yk1we7LEVkpWfZT05OU=\n',NULL),(2,'2012-11-01 14:24:50','2012-11-01 14:24:37','patrick','yQ7sfVQNBaEWRA/M1+/rtNp8yk1we7LEVkpWfZT05OU=\n',NULL);
/*!40000 ALTER TABLE `User` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2012-11-01 12:58:39
